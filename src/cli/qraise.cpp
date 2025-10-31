#include <string>
#include <fstream>
#include <regex>
#include <any>

#include <iostream>
#include <cstdlib>
#include <chrono>
#include <sstream>
#include <unistd.h>

#include "argparse.hpp"

#include "utils/constants.hpp"
#include "qraise/utils_qraise.hpp"
#include "qraise/args_qraise.hpp"
#include "qraise/noise_model_conf_qraise.hpp"
#include "qraise/simple_conf_qraise.hpp"
#include "qraise/cc_conf_qraise.hpp"
#include "qraise/qc_conf_qraise.hpp"
#include "qraise/infrastructure_conf_qraise.hpp"

#include "logger.hpp"

namespace {
    const int CORES_PER_NODE = 64; // For both QMIO and FT3
#ifdef CUNQA_SLURMLESS

    /// Generate a unique LOCAL job ID from timestamp
    std::string generate_slurmless_job_id()
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::ostringstream oss;
        oss << "local-" << epoch_ms << '-' << ::getpid();
        return oss.str();
    }
#endif
}

using namespace std::literals;
using namespace cunqa;

namespace {

void write_sbatch_header(std::ofstream& sbatchFile, const CunqaArgs& args) 
{
    // Escribir el contenido del script SBATCH
    sbatchFile << "#!/bin/bash\n";
    sbatchFile << "#SBATCH --job-name=qraise \n";

    int n_tasks = args.qc ? args.n_qpus * args.cores_per_qpu + args.n_qpus : args.n_qpus;    
    sbatchFile << "#SBATCH --ntasks=" << n_tasks << "\n";

    if (!args.qc) 
        sbatchFile << "#SBATCH -c " << args.cores_per_qpu << "\n";

    int n_nodes = number_of_nodes(args.n_qpus, args.cores_per_qpu, args.number_of_nodes.value(), CORES_PER_NODE, args.qc);
    sbatchFile << "#SBATCH -N " << n_nodes << "\n";
    
    if (args.qpus_per_node.has_value()) {
        if (args.n_qpus < args.qpus_per_node) {
            LOGGER_ERROR("Less qpus than selected qpus_per_node.");
            LOGGER_ERROR("\tNumber of QPUs: {}\n\t QPUs per node: {}", args.n_qpus, args.qpus_per_node.value());
            LOGGER_ERROR("Aborted.");
            std::system("rm qraise_sbatch_tmp.sbatch");
            return;
        } else {
            sbatchFile << "#SBATCH --ntasks-per-node=" << args.qpus_per_node.value() << "\n";
        }
    }

    if (args.node_list.has_value()) {
        if (args.number_of_nodes.value() != args.node_list.value().size()) {
            LOGGER_ERROR("Different number of node names than total nodes.");
            LOGGER_ERROR("\tNumber of nodes: {}\n\t Number of node names: {}", args.number_of_nodes.value(), args.node_list.value().size());
            LOGGER_ERROR("Aborted.");
            return;
        } else {
            sbatchFile << "#SBATCH --nodelist=";
            int comma = 0;
            for (auto& node_name : args.node_list.value()) {
                if (comma > 0 ) {
                    sbatchFile << ",";
                }
                sbatchFile << node_name;
                comma++;
            }
            sbatchFile << "\n";
        }
    }

    if (args.mem_per_qpu.has_value() && (args.mem_per_qpu.value()/args.cores_per_qpu > get_default_mem_per_core())) {
        LOGGER_ERROR("Too much memory per QPU. Please, decrease the mem-per-qpu or increase the cores-per-qpu.");
        return;
    }

    if (!args.qc) {
        if (args.mem_per_qpu.has_value() && check_mem_format(args.mem_per_qpu.value())) {
            sbatchFile << "#SBATCH --mem-per-cpu=" << args.mem_per_qpu.value()/args.cores_per_qpu << "G\n";
        } else if (args.mem_per_qpu.has_value() && !check_mem_format(args.mem_per_qpu.value())) {
            LOGGER_ERROR("Memory format is incorrect, must be: xG (where x is the number of Gigabytes).");
            return;
        } else if (!args.mem_per_qpu.has_value()) {
            int mem_per_core = get_default_mem_per_core();
            sbatchFile << "#SBATCH --mem-per-cpu=" << mem_per_core << "G\n";
        } 
    } else {
        if (args.mem_per_qpu.has_value() && check_mem_format(args.mem_per_qpu.value())) {
            sbatchFile << "#SBATCH --mem=" << args.mem_per_qpu.value() * args.n_qpus + args.n_qpus << "G\n";
        } else {
            int mem_per_core = get_default_mem_per_core();
            sbatchFile << "#SBATCH --mem=" << mem_per_core * args.cores_per_qpu * args.n_qpus + args.n_qpus << "G\n";
        }
    }


    if (check_time_format(args.time))
        sbatchFile << "#SBATCH --time=" << args.time << "\n";
    else {
        LOGGER_ERROR("Time format is incorrect, must be: xx:xx:xx.");
        return;
    }

    if (!check_simulator_name(args.simulator)){
        LOGGER_ERROR("Incorrect simulator name ({}).", args.simulator);
        return;
    }

    sbatchFile << "#SBATCH --output=qraise_%j\n";
}

void write_env_variables(std::ofstream& sbatchFile)
{
    auto store = std::getenv("STORE");

    sbatchFile << "\n";
    sbatchFile << "if [ ! -d \"$STORE/.cunqa\" ]; then\n";
    sbatchFile << "mkdir $STORE/.cunqa\n";
    sbatchFile << "fi\n";

    sbatchFile << "EPILOG_PATH=" << store << "/.cunqa/epilog.sh\n";
    sbatchFile << "export INFO_PATH=" << store << "/.cunqa/qpus.json\n";
    sbatchFile << "export COMM_PATH=" << store << "/.cunqa/communications.json\n";
}

std::string write_run_command(std::ofstream& sbatchFile, const CunqaArgs& args, const std::string& mode)
{
    std::string run_command;
    if (args.noise_properties.has_value() || args.fakeqmio.has_value()){
        LOGGER_DEBUG("noise_properties json path provided");
        if (args.simulator == "Munich" or args.simulator == "Cunqa"){
            LOGGER_WARN("Personalized noise models only supported for AerSimulator, switching simulator setting from {} to Aer.", args.simulator.c_str());
        }
        if (args.cc || args.qc){
            LOGGER_ERROR("Personalized noise models not supported for classical/quantum communications schemes.");
            return "";
        }

        if (args.backend.has_value()){
            LOGGER_WARN("Because noise properties were provided backend will be redefined according to them.");
        }

        run_command = get_noise_model_run_command(args, mode);


    } else if ((!args.noise_properties.has_value() || !args.fakeqmio.has_value()) && (args.no_thermal_relaxation || args.no_gate_error || args.no_readout_error)){
        LOGGER_ERROR("noise_properties flags where provided but --noise_properties nor --fakeqmio args were not included.");
        return "";

    } else {
        if (args.cc) {
            LOGGER_DEBUG("Classical communications");
            run_command = get_cc_run_command(args, mode);
        } else if (args.qc) {
            LOGGER_DEBUG("Quantum communications");
            run_command = get_qc_run_command(args, mode);
        } else {
            LOGGER_DEBUG("No communications");
            run_command = get_simple_run_command(args, mode);
        }
    }

    LOGGER_DEBUG("Run command: {}", run_command);
    sbatchFile << run_command;
    return run_command;
}

}


int main(int argc, char* argv[]) 
{
    auto args = argparse::parse<CunqaArgs>(argc, argv, true); //true ensures an error is raised if we feed qraise an unrecognized flag
    const char* store = std::getenv("STORE");
    if (!store) {
        LOGGER_ERROR("STORE environment variable is not defined; no runtime directory.");
        return -1;
    }
    std::string info_path = std::string(store) + "/.cunqa/qpus.json";

    if (args.infrastructure.has_value()) {
#ifdef CUNQA_SLURMLESS
        LOGGER_ERROR("Not supported without Slurm.");
        return -1;
#else
        LOGGER_DEBUG("Raising infrastructure");
        std::ofstream sbatchFile("qraise_sbatch_tmp.sbatch");
        write_sbatch_file_from_infrastructure(sbatchFile, args);
        sbatchFile.close();
        std::system("sbatch qraise_sbatch_tmp.sbatch");
        std::system("rm qraise_sbatch_tmp.sbatch");
        return 0;
#endif
    }

    std::string mode = args.cloud ? "cloud" : "hpc";
    std::string family = args.family_name;
    if (exists_family_name(family, info_path)) { //Check if there exists other QPUs with same family name
        LOGGER_ERROR("There are QPUs with the same family name as the provided: {}.", family);
#ifndef CUNQA_SLURMLESS
        std::system("rm qraise_sbatch_tmp.sbatch");
#endif
        return -1;
    }

#ifndef CUNQA_SLURMLESS
    std::ofstream sbatchFile("qraise_sbatch_tmp.sbatch");
    write_sbatch_header(sbatchFile, args);
    write_env_variables(sbatchFile);
    std::string run_command = write_run_command(sbatchFile, args, mode);
    sbatchFile.close();

    if (run_command.empty()) {
        LOGGER_ERROR("Aborting qraise: invalid run_command.");
        std::system("rm qraise_sbatch_tmp.sbatch");
        return -1;
    }

    std::system("sbatch qraise_sbatch_tmp.sbatch");
    std::system("rm qraise_sbatch_tmp.sbatch");
#else
    const int total_tasks = args.qc ? args.n_qpus * args.cores_per_qpu + args.n_qpus : args.n_qpus;
    const std::string job_id = generate_slurmless_job_id();

    std::ofstream runFile("qraise_run_tmp.sh");
    runFile << "#!/bin/bash\n";
    runFile << "set -e\n";
    write_env_variables(runFile);
    runFile << "export CUNQA_JOB_ID=\"" << job_id << "\"\n";
    runFile << "export SLURM_JOB_ID=\"" << job_id << "\"\n";
    runFile << "export SLURM_NTASKS=\"" << total_tasks << "\"\n";
    std::string run_command = write_run_command(runFile, args, mode);
    runFile.close();

    if (run_command.empty()) {
        LOGGER_ERROR("Aborting qraise: invalid run_command.");
        (void)std::system("rm qraise_run_tmp.sh");
        return -1;
    }

    int ret = std::system("bash qraise_run_tmp.sh");
    if (ret != 0) {
        LOGGER_ERROR("qraise run script exited with status {}.", ret);
    }
    (void)std::system("rm qraise_run_tmp.sh");
#endif

    return 0;
}
