#include <string>
#include <fstream>
#include <regex>
#include <any>
#include <nlohmann/json.hpp>
#include <iostream>

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
    auto args = argparse::parse<MyArgs>(argc, argv);
    std::ofstream sbatchFile("qraise_sbatch_tmp.sbatch");
    SPDLOG_LOGGER_DEBUG(logger, "Temporal file qraise_sbatch_tmp.sbatch created.");
    std::string run_command;

    // Escribir el contenido del script SBATCH
    sbatchFile << "#!/bin/bash\n";
    sbatchFile << "#SBATCH --job-name=qraise \n";
    sbatchFile << "#SBATCH -c 2 \n";
    sbatchFile << "#SBATCH --ntasks=" << args.n_qpus << "\n";
    sbatchFile << "#SBATCH -N 1 \n";
    // sbatchFile << "#SBATCH --nodelist=c7-" << args.node << "\n"; //Alvaro

    // TODO: Can the user decide the number of cores?
    if (check_mem_format(args.mem_per_qpu)){
        int mem_per_qpu = args.mem_per_qpu[0] - '0';
        sbatchFile << "#SBATCH --mem-per-cpu=" << mem_per_qpu*2 << "G\n";
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

    sbatchFile << "export INFO_PATH=" << std::getenv("STORE") << "/.cunqa/qpus.json\n";


    if ( (args.comm.value() != "no_comm") && (args.comm.value() != "class_comm") && (args.comm.value() != "quantum_comm")) {
        SPDLOG_LOGGER_ERROR(logger, "--comm only admits \"no_comm\", \"class_comm\" or \"quantum_comm\" as valid arguments");
        std::system("rm qraise_sbatch_tmp.sbatch");
        return 0;
    }

    if (args.fakeqmio.has_value()) {
        SPDLOG_LOGGER_DEBUG(logger, "Fakeqmio provided as a FLAG");
        run_command = get_fakeqmio_run_command(args);
    } else {
        if (args.comm.value() == "no_comm") {
            SPDLOG_LOGGER_DEBUG(logger, "No communications");
            run_command = get_no_comm_run_command(args);
            if (run_command == "0") {
                return 0;
            }
        } else if (args.comm.value() == "class_comm") {
            SPDLOG_LOGGER_DEBUG(logger, "Classical communications");
            run_command = get_class_comm_run_command(args);
            if (run_command == "0") {
                return 0;
            }
        } else { //Quantum Communication
            SPDLOG_LOGGER_ERROR(logger, "Quantum communications are not implemented yet");
            std::system("rm qraise_sbatch_tmp.sbatch");
            return 0;
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