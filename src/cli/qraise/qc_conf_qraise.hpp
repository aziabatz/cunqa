#pragma once

#include "argparse.hpp"
#include "utils/constants.hpp"
#include "args_qraise.hpp"
#include "utils_qraise.hpp"
#include "logger.hpp"
#include <cstdlib>

std::string get_qc_run_command(const CunqaArgs& args, const std::string& mode)
{
    std::string run_command;
    std::string subcommand;
    std::string backend_path;
    std::string backend;

    if (args.simulator != "Aer" && args.simulator != "Munich" && args.simulator != "Cunqa") {
        LOGGER_ERROR("Quantum communications only are available under \"Aer\", \"Munich\" and \"CUNQA\" simulators, but the following simulator was provided: {}", args.simulator);
        std::system("rm qraise_sbatch_tmp.sbatch");
        return "0";
    } 

    subcommand = mode + " qc " + args.family_name + " " + args.simulator;
    LOGGER_DEBUG("Qraise with quantum communications and default backend. \n");

    int num_ports = args.n_qpus * 3;
    int simulator_n_cores = args.cores_per_qpu * args.n_qpus; 
    int simulator_memory = args.mem_per_qpu.has_value() ? args.mem_per_qpu.value() * args.n_qpus : get_default_mem_per_core() * args.cores_per_qpu * args.n_qpus;
    
#ifdef CUNQA_SLURMLESS
    std::string qpu_mpi_cmd = "mpirun --allow-run-as-root -np " + std::to_string(args.n_qpus);
    if (const char* extra = std::getenv("CUNQA_MPI_FLAGS")) {
        qpu_mpi_cmd += " ";
        qpu_mpi_cmd += extra;
    }
    std::string qpu_port_range = allocate_port_range(num_ports);
    run_command =  "CUNQA_PORT_RANGE=" + qpu_port_range + " SLURM_STEP_RESV_PORTS=" + qpu_port_range + " " + qpu_mpi_cmd + " setup_qpus $INFO_PATH " +  subcommand + " &\n";

    run_command += "sleep 1\n";

    std::string executor_mpi_cmd = "mpirun --allow-run-as-root -np 1";
    if (const char* extra = std::getenv("CUNQA_MPI_FLAGS")) {
        executor_mpi_cmd += " ";
        executor_mpi_cmd += extra;
    }
    std::string executor_port_range = allocate_port_range(args.n_qpus);
    run_command +=  "CUNQA_PORT_RANGE=" + executor_port_range + " SLURM_STEP_RESV_PORTS=" + executor_port_range + " " + executor_mpi_cmd + " setup_executor " + args.simulator + " " + args.family_name + "\n";
#else
    run_command =  "srun -n " + std::to_string(args.n_qpus) + " -c 1 --mem-per-cpu=1G --resv-ports=" + std::to_string(num_ports) + " --exclusive --task-epilog=$EPILOG_PATH setup_qpus $INFO_PATH " +  subcommand + " &\n";

    // This is done to avoid run conditions in the IP publishing of the QPUs for the executor
    run_command += "sleep 1\n";
    run_command +=  "srun -n 1 -c " + std::to_string(simulator_n_cores) + " --mem=" + std::to_string(simulator_memory) + "G --resv-ports=" + std::to_string(args.n_qpus) + " --exclusive setup_executor " + args.simulator + " " + args.family_name + "\n";
#endif

    #ifndef USE_ZMQ_BTW_QPU
        LOGGER_ERROR("For quantum communications ZMQ has to be available.");
        return "0";
    #endif

    return run_command;
}
