#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <chrono>
#include <optional>
#include <mpi.h>
#include "zmq.hpp"
#include <variant>

#include "utils/setup_qpus_utils.hpp"


using json = nlohmann::json;


int main(int argc, char *argv[])
{
    SPDLOG_LOGGER_DEBUG(logger,"Setup QPUs with argc={} arguments", argc);
    std::string info_path(argv[1]);
    std::string communications(argv[2]);
    std::string simulator(argv[3]);
    std::string backend_str;
    json backend_json;
    ZMQSockets zmq_sockets;
    
    json qpu_config_json = {};
    try {
        if (argc == 5) {
            backend_str = std::string(argv[4]);
            SPDLOG_LOGGER_DEBUG(logger, "A PATH to a backend was provided: {}", backend_str);
            backend_json = json::parse(backend_str);
            qpu_config_json = get_qpu_config(backend_json);
            SPDLOG_LOGGER_DEBUG(logger, "Json with QPU configuration created");
            
        } else  if (argc < 4) {
            SPDLOG_LOGGER_ERROR(logger, "Not a QPU configuration was given.");
        }

        if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
            if (search->second == SimType::Aer) {
                SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with AerSimulator.");
                turn_on_qpu<SimType::Aer>(qpu_config_json, info_path, communications, argc, argv);

            } else if (search->second == SimType::Munich) {
                SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with MunichSimulator.");
                turn_on_qpu<SimType::Munich>(qpu_config_json, info_path, communications, argc, argv);

            } else if (search->second == SimType::Cunqa) {
                SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with CunqaSimulator.");
                turn_on_qpu<SimType::Cunqa>(qpu_config_json, info_path, communications, argc, argv);
            }
        }

        
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning ON the QPUs because: \n\t{}", e.what());
    }

    return 0;
    
}