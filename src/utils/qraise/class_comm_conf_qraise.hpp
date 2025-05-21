#pragma once

#include <string>
#include <any>

#include "argparse.hpp"
#include "utils/constants.hpp"
#include "args_qraise.hpp"
#include "logger.hpp"


std::string get_class_comm_run_command(auto& args, std::string& mode)
{
    std::string run_command;
    std::string subcommand;
    std::string backend_path;
    std::string backend;

    if (std::any_cast<std::string>(args.simulator) != "Cunqa") {
        LOGGER_ERROR("Classical communications only are available under \"Cunqa\" simulator, but the following simulator was provided: {}", std::any_cast<std::string>(args.simulator));
        std::system("rm qraise_sbatch_tmp.sbatch");
        return "0";
    } 

    if (args.backend.has_value()) {
        backend_path = std::any_cast<std::string>(args.backend.value());
        backend = R"({"backend_path":")" + backend_path + R"("})" ;
        subcommand = mode + " class_comm " + std::any_cast<std::string>(args.family) + " " + std::any_cast<std::string>(args.simulator) + " \'" + backend + "\'" "\n";
        LOGGER_DEBUG("Qraise with classical communications and personalized CunqaSimulator backend. \n");
    } else {
        subcommand = mode + " class_comm " + std::any_cast<std::string>(args.family) + " " + std::any_cast<std::string>(args.simulator) + "\n";
        LOGGER_DEBUG("Qraise with classical communications and default CunqaSimulator backend. \n");
    }

    #ifdef QPU_MPI
    run_command =  "srun --mpi=pmix --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " +  subcommand;
    LOGGER_DEBUG("Run command with MPI comm: {}", run_command);
    #endif

    #ifdef QPU_ZMQ
    int num_ports = args.n_qpus * 2;
    run_command =  "srun --resv-ports=" + std::to_string(num_ports) + " --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " +  subcommand;
    LOGGER_DEBUG("Run command with ZMQ comm: {}", run_command);
    #endif

    return run_command;
}