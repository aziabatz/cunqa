#pragma once

#include <string>
#include <any>
#include <cstdlib>

#include "argparse.hpp"
#include "utils/constants.hpp"
#include "args_qraise.hpp"
#include "logger.hpp"
#include "utils_qraise.hpp"


std::string get_cc_run_command(const CunqaArgs& args, const std::string& mode)
{
    std::string run_command;
    std::string subcommand;
    std::string backend_path;
    std::string backend;

    if (std::any_cast<std::string>(args.simulator) != "Cunqa" && std::any_cast<std::string>(args.simulator) != "Munich" && std::any_cast<std::string>(args.simulator) != "Aer") {
        LOGGER_ERROR("Classical communications only are available under \"Cunqa\", \"Munich\" and \"Aer\" simulators, but the following simulator was provided: {}", std::any_cast<std::string>(args.simulator));
        std::system("rm qraise_sbatch_tmp.sbatch");
        return "0";
    } 

    if (args.backend.has_value()) {
        backend_path = std::any_cast<std::string>(args.backend.value());
        backend = R"({"backend_path":")" + backend_path + R"("})" ;
        subcommand = mode + " cc " + std::any_cast<std::string>(args.family_name) + " " + std::any_cast<std::string>(args.simulator) + " \'" + backend + "\'" "\n";
        LOGGER_DEBUG("Qraise with classical communications and personalized CunqaSimulator backend. \n");
    } else {
        subcommand = mode + " cc " + std::any_cast<std::string>(args.family_name) + " " + std::any_cast<std::string>(args.simulator) + "\n";
        LOGGER_DEBUG("Qraise with classical communications and default CunqaSimulator backend. \n");
    }

#ifdef USE_MPI_BTW_QPU
#ifdef CUNQA_SLURMLESS
    std::string mpi_cmd = "mpirun --allow-run-as-root -np " + std::to_string(args.n_qpus);
    if (const char* extra = std::getenv("CUNQA_MPI_FLAGS")) {
        mpi_cmd += " ";
        mpi_cmd += extra;
    }
    run_command = mpi_cmd + " setup_qpus $INFO_PATH " +  subcommand;
#else
    run_command =  "srun --mpi=pmix --task-epilog=$EPILOG_PATH setup_qpus $INFO_PATH " +  subcommand;
#endif
    LOGGER_DEBUG("Run command with MPI comm: {}", run_command);
#endif

#ifdef USE_ZMQ_BTW_QPU
    int num_ports = args.n_qpus * 2;
#ifdef CUNQA_SLURMLESS
    std::string port_range = allocate_port_range(num_ports);
    std::string mpi_cmd = "mpirun --allow-run-as-root -np " + std::to_string(args.n_qpus);
    if (const char* extra = std::getenv("CUNQA_MPI_FLAGS")) {
        mpi_cmd += " ";
        mpi_cmd += extra;
    }
    run_command = "CUNQA_PORT_RANGE=" + port_range + " SLURM_STEP_RESV_PORTS=" + port_range + " " + mpi_cmd + " setup_qpus $INFO_PATH " +  subcommand;
#else
    run_command =  "srun --resv-ports=" + std::to_string(num_ports) + " --task-epilog=$EPILOG_PATH setup_qpus $INFO_PATH " +  subcommand;
#endif
    LOGGER_DEBUG("Run command with ZMQ comm: {}", run_command);
#endif

    return run_command;
}
