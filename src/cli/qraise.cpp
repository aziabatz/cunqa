#include <string>
#include <fstream>
#include <regex>
#include <any>
#include <nlohmann/json.hpp>
#include <iostream>

#include "argparse.hpp"
#include "logger/logger.hpp"
#include "constants.hpp"

using json = nlohmann::json;

using namespace std::literals;

struct MyArgs : public argparse::Args 
{
    //int& node = kwarg("node", "Specific node to raise the qpus."); //Alvaro 
    int& n_qpus                          = kwarg("n,num_qpus", "Number of QPUs to be raised.");
    std::string& time                    = kwarg("t,time", "Time for the QPUs to be raised.");
    std::optional<std::string>& backend  = kwarg("b,backend", "Path to the backend config file.");
    std::string& simulator               = kwarg("sim,simulator", "Simulator reponsible of running the simulations.").set_default("Aer");
    std::string& mem_per_qpu             = kwarg("mem-per-qpu", "Memory given to each QPU.").set_default("7G");
    std::optional<std::string>& fakeqmio = kwarg("fq,fakeqmio", "Raise FakeQmio backend from calibration file", /*implicit*/"last_calibrations");

    void welcome() {
        std::cout << "Welcome to qraise command, a command responsible for turn on the required QPUs.\n" << std::endl;
    }
};

bool check_time_format(const std::string& time)
{
    std::regex format("^(\\d{2}):(\\d{2}):(\\d{2})$");
    return std::regex_match(time, format);   
}

bool check_mem_format(const std::string& mem) 
{
    std::regex format("^(\\d{1,2})G$");
    return std::regex_match(mem, format);
}

int main(int argc, char* argv[]) 
{
    srand((unsigned int)time(NULL));
    int intSEED = rand() % 1000;
    std::string SEED = std::to_string(intSEED);
    auto args = argparse::parse<MyArgs>(argc, argv);
    std::ofstream sbatchFile("qraise_sbatch_tmp.sbatch");
    SPDLOG_LOGGER_DEBUG(logger, "Temporal file qraise_sbatch_tmp.sbatch created.");

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
    sbatchFile << "if [ ! -d \"$STORE/.api_simulator\" ]; then\n";
    sbatchFile << "mkdir $STORE/.api_simulator\n";
    sbatchFile << "fi\n";

    const char* var_name = "INSTALL_PATH"; // Replace with your variable name
    const char* var_install_path = std::getenv(var_name);

    if (var_install_path) {
        sbatchFile << "BINARIES_DIR=" << var_install_path << "/bin\n";
    } else {
        std::cerr << "Environment variable INSTALL_PATH is not set: aborting.\n"; 
    }

    sbatchFile << "export INFO_PATH=" << std::getenv("STORE") << "/.api_simulator/qpus.json\n";

    std::string backend_path;
    json backend_json = {};
    std::string backend;
    if (args.fakeqmio.has_value()) {
        backend_path = std::any_cast<std::string>(args.fakeqmio.value());
        backend_json = {
            {"fakeqmio_path", backend_path}
        };
        backend = R"({"fakeqmio_path":")" + backend_path + R"("})" ;
        sbatchFile << "srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " << args.simulator.c_str() << " " << "\'"<< backend << "\'" << "\n";  
        SPDLOG_LOGGER_DEBUG(logger, "FakeQmio. Command: srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH {} {}\n", args.simulator.c_str(), backend);
    }
    
    if (args.backend.has_value()) {
        if(args.backend.value() == "etiopia_computer.json") {
            SPDLOG_LOGGER_ERROR(logger, "Terrible mistake. Possible solution: {}", cafe);
            std::system("rm qraise_sbatch_tmp.sbatch");
            return 0;
        } else {
            backend_path = std::any_cast<std::string>(args.backend.value());
            backend_json = {
                {"backend_path", backend_path}
            };
            backend = R"({"backend_path":")" + backend_path + R"("})" ;
            sbatchFile << "srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " << args.simulator.c_str() << " " << "\'"<< backend << "\'" << "\n";
            SPDLOG_LOGGER_DEBUG(logger, "Qraise with backend. Command: srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH {} {}\n", args.simulator.c_str(), backend);
        }
    } else {
        sbatchFile << "srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " << args.simulator.c_str() << "\n";
        SPDLOG_LOGGER_DEBUG(logger, "Command: srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH {} \n", args.simulator.c_str());
    }

    sbatchFile.close();

    std::system("sbatch qraise_sbatch_tmp.sbatch");
    std::system("rm qraise_sbatch_tmp.sbatch");

    SPDLOG_LOGGER_DEBUG(logger, "Sbatch launched and qraise_sbatch_tmp.sbatch removed.");

    return 0;
}