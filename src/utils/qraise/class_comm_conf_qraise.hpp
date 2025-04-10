#pragma once

#include <string>
#include <any>

#include "argparse.hpp"
#include "constants.hpp"
#include "logger/logger.hpp"
#include "args_qraise.hpp"


std::string get_class_comm_run_command(auto& args)
{
    std::string run_command;
    std::string subcommand;
    std::string backend_path;
    std::string backend;

    if (std::any_cast<std::string>(args.simulator) != "Cunqa") {
        SPDLOG_LOGGER_ERROR(logger, "Classical communications only are available under \"Cunqa\" simulator, but the following simulator was provided: {}", std::any_cast<std::string>(args.simulator));
        std::system("rm qraise_sbatch_tmp.sbatch");
        return "0";
    } 

    if (args.backend.has_value()) {
        backend_path = std::any_cast<std::string>(args.backend.value());
        backend = R"({"backend_path":")" + backend_path + R"("})" ;
        subcommand = "class_comm " + std::any_cast<std::string>(args.simulator) + " " + backend + "\n";
        SPDLOG_LOGGER_DEBUG(logger, "Qraise with classical communications and personalized CunqaSimulator backend. \n");
    } else {
        subcommand = "class_comm " + std::any_cast<std::string>(args.simulator) + "\n";
        SPDLOG_LOGGER_DEBUG(logger, "Qraise with classical communications and default CunqaSimulator backend. \n");
    }

    #ifdef QPU_MPI
    run_command =  "srun --mpi=pmix --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " +  subcommand;
    SPDLOG_LOGGER_DEBUG(logger, "Run command with MPI comm: {}", run_command );
    #endif

    #ifdef QPU_ZMQ
    int num_ports = args.n_qpus * 2;
    run_command =  "srun --resv-ports=" + std::to_string(num_ports) + " --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " +  subcommand;
    SPDLOG_LOGGER_DEBUG(logger, "Run command with ZMQ comm: {}", run_command);
    #endif

    return run_command;
}