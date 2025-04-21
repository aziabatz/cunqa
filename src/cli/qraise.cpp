#include <string>
#include <fstream>
#include <regex>
#include <any>
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstdlib>

#include "argparse.hpp"
#include "logger/logger.hpp"
#include "constants.hpp"
#include "utils/qraise/utils_qraise.hpp"
#include "utils/qraise/args_qraise.hpp"
#include "utils/qraise/fakeqmio_conf_qraise.hpp"
#include "utils/qraise/no_comm_conf_qraise.hpp"
#include "utils/qraise/class_comm_conf_qraise.hpp"
#include "utils/qraise/quantum_comm_conf_qraise.hpp"

using json = nlohmann::json;


using namespace std::literals;

int main(int argc, char* argv[]) 
{
    srand((unsigned int)time(NULL));
    int intSEED = rand() % 1000;
    std::string SEED = std::to_string(intSEED);

    auto args = argparse::parse<MyArgs>(argc, argv, true); //true ensures an error is raised if we feed qraise an unrecognized flag

    const char* store = std::getenv("STORE");
    std::string info_path = std::string(store) + "/.cunqa/qpus.json";

    //Checking mode:
    if ((args.mode != "hpc") && (args.mode != "cloud")) {
        std::cerr << "\033[1;31m" << "Error: " << "mode argument different than hpc or cloud. Given: " << args.mode << "\n";
        std::cerr << "Aborted." << "\033[0m \n";
        return 1;
    }

    std::ofstream sbatchFile("qraise_sbatch_tmp.sbatch");
    SPDLOG_LOGGER_DEBUG(logger, "Temporal file qraise_sbatch_tmp.sbatch created.");
    std::string run_command;

    // Escribir el contenido del script SBATCH
    sbatchFile << "#!/bin/bash\n";
    sbatchFile << "#SBATCH --job-name=qraise \n";
    sbatchFile << "#SBATCH -c " << args.cores_per_qpu << "\n";
    sbatchFile << "#SBATCH --ntasks=" << args.n_qpus << "\n";
    sbatchFile << "#SBATCH -N " << args.number_of_nodes.value() << "\n";

    if (args.qpus_per_node.has_value()) {
        if (args.n_qpus < args.qpus_per_node) {
            std::cerr << "\033[1;31m" << "Error: " << "Less qpus than selected qpus_per_node.\n";
            std::cerr << "Number of QPUs: " << args.n_qpus << " QPUs per node: " << args.qpus_per_node.value() << "\n";
            std::cerr << "Aborted." << "\033[0m \n";
            std::system("rm qraise_sbatch_tmp.sbatch");
            return 1;
        } else {
            sbatchFile << "#SBATCH --ntasks-per-node=" << args.qpus_per_node.value() << "\n";
        }
    }

    if (args.node_list.has_value()) {
        if (args.number_of_nodes.value() != args.node_list.value().size()) {
            std::cerr << "\033[1;31m" << "Error: " << "Different number of node names than total nodes.\n";
            std::cerr << "Number of nodes: " << args.number_of_nodes.value() << " Number of node names: " << args.node_list.value().size() << "\n";
            std::cerr << "Aborted." << "\033[0m \n";
            std::system("rm qraise_sbatch_tmp.sbatch");
            return 1;
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

    if (check_mem_format(args.mem_per_qpu)){
        int mem_per_qpu = args.mem_per_qpu;
        int cores_per_qpu = args.cores_per_qpu;
        sbatchFile << "#SBATCH --mem-per-cpu=" << mem_per_qpu/cores_per_qpu << "G\n";
    } else {
        SPDLOG_LOGGER_DEBUG(logger, "Memory format is incorrect, must be: xG (where x is the number of Gigabytes).");
        return -1;
    }

    if (check_time_format(args.time))
        sbatchFile << "#SBATCH --time=" << args.time << "\n";
    else {
        SPDLOG_LOGGER_DEBUG(logger, "Time format is incorrect, must be: xx:xx:xx.");
        return -1;
    }

    int memory_specs = check_memory_specs(args.mem_per_qpu, args.cores_per_qpu);

    if (memory_specs == 1) {
        SPDLOG_LOGGER_ERROR(logger, "Too much memory per QPU in QMIO. Please, decrease the mem-per-QPU or increase the cores-per-qpu. (Max mem-per-cpu = 16)");
        return -1;
    } else if (memory_specs == 2) {
        SPDLOG_LOGGER_ERROR(logger, "Too much memory per QPU in FT3. Please, decrease the mem-per-QPU or increase the cores-per-qpu. Max mem-per-cpu = 4");
        return -1;
    }

    sbatchFile << "#SBATCH --output=qraise_%j\n";

    sbatchFile << "\n";
    sbatchFile << "if [ ! -d \"$STORE/.cunqa\" ]; then\n";
    sbatchFile << "mkdir $STORE/.cunqa\n";
    sbatchFile << "fi\n";

    const char* var_name = "INSTALL_PATH"; // Replace with your variable name
    const char* var_install_path = std::getenv(var_name);

    if (var_install_path) {
        sbatchFile << "BINARIES_DIR=" << var_install_path << "/bin\n";
    } else {
        std::cerr << "Environment variable INSTALL_PATH is not set: aborting.\n"; 
    }

    
    sbatchFile << "export INFO_PATH=" << info_path + "\n";

    std::string family_name = std::any_cast<std::string>(args.family_name);
    if (exists_family_name(family_name, info_path)) { //Check if there exists other QPUs with same family name
        SPDLOG_LOGGER_ERROR(logger, "There are QPUs with the same family name as the provided: {}.", family_name);
        std::system("rm qraise_sbatch_tmp.sbatch");
        return 0;
    }

    if (args.fakeqmio.has_value()) {
        SPDLOG_LOGGER_DEBUG(logger, "Fakeqmio provided as a FLAG");
        run_command = get_fakeqmio_run_command(args);
    } else {
        if (args.classical_comm) {
            SPDLOG_LOGGER_DEBUG(logger, "Classical communications");
            run_command = get_class_comm_run_command(args);
            if (run_command == "0") {
                return 0;
            }
        } else if (args.quantum_comm) {
            SPDLOG_LOGGER_ERROR(logger, "Quantum communications are not implemented yet");
            std::system("rm qraise_sbatch_tmp.sbatch");
            return 0;
        } else {
            SPDLOG_LOGGER_DEBUG(logger, "No communications");
            run_command = get_no_comm_run_command(args);
            if (run_command == "0") {
                return 0;
            }
        }
    }

    SPDLOG_LOGGER_DEBUG(logger, "Run command: ", run_command);
    sbatchFile << run_command;

    sbatchFile.close();

    std::system("sbatch qraise_sbatch_tmp.sbatch");
    std::system("rm qraise_sbatch_tmp.sbatch");

    SPDLOG_LOGGER_DEBUG(logger, "Sbatch launched and qraise_sbatch_tmp.sbatch removed.");

    return 0;
}