#pragma once

#include <string>
#include <any>

#include "argparse.hpp"
#include "constants.hpp"
#include "logger/logger.hpp"
#include "args_qraise.hpp"

std::string get_noise_model_run_command(auto& args, std::string& mode)
{
    std::string run_command;
    std::string subcommand;
    std::string properties_path;
    std::string properties;
    int thermal_relaxation;
    int readout_error;
    int gate_error;

    if (args.no_thermal_relaxation){
        thermal_relaxation = 0;
    }else{
        thermal_relaxation = 1;
    }

    if (args.no_readout_error){
        readout_error = 0;
    }else{
        readout_error = 1;
    }

    if (args.no_gate_error){
        gate_error = 0;
    }else{
        gate_error = 1;
    }

    properties_path = std::any_cast<std::string>(args.properties.value());

    properties = R"({"properties_path":")" + properties_path
               + R"(","thermal_relaxation":")" +  std::to_string(thermal_relaxation)
               + R"(","readout_error":")" +  std::to_string(readout_error)
               + R"(","gate_error":")" +  std::to_string(gate_error)+ R"("})" ;

    subcommand = mode + " no_comm " + std::any_cast<std::string>(args.family_name) + " Aer \'" + backend + "\'" + "\n";
    run_command =  "srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " + subcommand;
    SPDLOG_LOGGER_DEBUG(logger, "Qraise FakeQmio. \n");
    SPDLOG_LOGGER_DEBUG(logger, "Run command: {}", run_command);

    return run_command;
}