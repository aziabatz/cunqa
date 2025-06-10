#pragma once

#define USE_ZMQ_BTW_QPU

#include "argparse.hpp"
#include "utils/constants.hpp"
#include "args_qraise.hpp"
#include "logger.hpp"

std::string get_qc_run_command(const CunqaArgs& args, std::string& mode)
{
    std::string run_command;
    std::string subcommand;
    std::string backend_path;
    std::string backend;

    if (args.simulator != "Cunqa" && args.simulator != "Munich") {
        LOGGER_ERROR("Quantum communications only are available under \"Munich\" simulators, but the following simulator was provided: {}", args.simulator);
        std::system("rm qraise_sbatch_tmp.sbatch");
        return "0";
    } 

    subcommand = mode + " quantum_comm " + args.family_name + " " + args.simulator + "\n";
    LOGGER_DEBUG("Qraise with quantum communications and default CunqaSimulator backend. \n");

    #ifdef USE_ZMQ_BTW_QPU
    int num_ports = args.n_qpus * 3;
    run_command =  "srun -n=" + args.n_qpus + 
                        "--cpus-per-task=1 --resv-ports=" + std::to_string(num_ports) + " --task-epilog=$EPILOG_PATH setup_quantum_sim $INFO_PATH " +  subcommand;
    LOGGER_DEBUG("Run command with ZMQ comm: {}", run_command);

    // This is done to avoid run conditions in the IP publishing of the QPUs for the executor
    run_command += "sleep 0.1"

    auto simulation_cores = args.cores_per_qpu * args.n_qpus - args.n_qpus;
    run_command +=  "srun -n=1 --cpus-per-task=" + simulation_cores + 
                        " --resv-ports=" + args.n_qpus + " setup_executor " + args.simulator + " " + args.family_name;
    #else
    LOGGER_ERROR("For quantum communications ZMQ has to be available.");
    return "0";
    #endif

    return run_command;
}