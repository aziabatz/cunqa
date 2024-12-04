#include "argparse.hpp"
#include "simulators/simulator.hpp"
#include <string>
#include <regex>

using namespace std::literals;

struct MyArgs : public argparse::Args {
    int& n_qpus                         = kwarg("n,num_qpus", "Number of QPUs to be raised.");
    std::string& time                   = kwarg("t,time", "Time for the QPUs to be raised.");
    std::optional<std::string>& backend = kwarg("back,backend_path", "Path to the backend config file.");
    std::string& simulator              = kwarg("sim,simulator", "Simulator reponsible of running the simulations.").set_default("Aer"); 
    std::string& config_path            = kwarg("conf,config_path", "Path for saving the QPUs configurations.").set_default(std::getenv("STORE") + "/.api_simulator/qpu.json"s);


    void welcome() {
        std::cout << "Welcome to qraise command, a command responsible for turn on the required QPUs.\n" << std::endl;
    }
};

bool check_time_format(const std::string& time)
{
    std::regex format("^(\\d{2}):(\\d{2}):(\\d{2})$");
    return std::regex_match(time, format);   
}

int main(int argc, char* argv[]) {
    auto args = argparse::parse<MyArgs>(argc, argv);

    std::ofstream sbatchFile("qraise_sbatch_tmp.sbatch");

    // Escribir el contenido del script SBATCH
    sbatchFile << "#!/bin/bash\n";
    sbatchFile << "#SBATCH --job-name=qraise \n";
    sbatchFile << "#SBATCH -c 2 \n";
    sbatchFile << "#SBATCH --ntasks=" << args.n_qpus << "\n";

    if (check_time_format(args.time))
        sbatchFile << "#SBATCH --time=" << args.time << "\n";
    else
        std::cerr << "ERROR: Time format is incorrect, must be: XX:XX:XX\n";
    sbatchFile << "#SBATCH --output=qraise_%j\n";
    sbatchFile << "#SBATCH --mem-per-cpu=1G\n";


    sbatchFile << "\n";
    sbatchFile << "if [ ! -d \"$STORE/.api_simulator\" ]; then\n";
    sbatchFile << "mkdir $STORE/.api_simulator\n";
    sbatchFile << "fi\n";
    sbatchFile << "CURRENT_DIR=$(pwd)\n";
    sbatchFile << "export CONFIG_PATH=" << args.config_path.c_str() << "\n";
    if(args.backend.has_value())
        sbatchFile << "srun --task-epilog=$CURRENT_DIR/epilog.sh setup_qpus $CONFIG_PATH " << args.simulator.c_str() << " " << args.backend.value().c_str() << "\n";
    else
        sbatchFile << "srun --task-epilog=$CURRENT_DIR/epilog.sh setup_qpus $CONFIG_PATH " << args.simulator.c_str() << "\n";
    sbatchFile << "rm -rf $STORE/.api_simulator\n";
    sbatchFile.close();

    std::system("sbatch qraise_sbatch_tmp.sbatch");
    std::system("rm qraise_sbatch_tmp.sbatch");

    return 0;
}