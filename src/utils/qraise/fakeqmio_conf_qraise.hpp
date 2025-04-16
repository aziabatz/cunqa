#pragma once

#include <string>
#include <any>

#include "argparse.hpp"
#include "constants.hpp"
#include "logger/logger.hpp"
#include "args_qraise.hpp"

std::string get_fakeqmio_run_command(auto& args)
{
    std::string run_command;
    std::string subcommand;
    std::string backend_path;
    std::string backend;

    backend_path = std::any_cast<std::string>(args.fakeqmio.value());
    backend = R"({"fakeqmio_path":")" + backend_path + R"("})" ;
    subcommand = std::any_cast<std::string>(args.mode) + " no_comm " + std::any_cast<std::string>(args.family_name) + " Aer \'" + backend + "\'" + "\n";
    run_command =  "srun --task-epilog=$BINARIES_DIR/epilog.sh setup_qpus $INFO_PATH " + subcommand;
    SPDLOG_LOGGER_DEBUG(logger, "Qraise FakeQmio. \n");
    SPDLOG_LOGGER_DEBUG(logger, "Run command: {}", run_command);

    return run_command;
}