#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <chrono>
#include "qpu.hpp"
#include "simulators/simulator.hpp"
#include "config/qpu_config.hpp"
#include "comm/server.hpp"
#include "comm/client.hpp"
#include "utils/logger.hpp"

using json = nlohmann::json;

int main(int argc, char *argv[])
{
    SPDLOG_LOGGER_DEBUG(loggie::logger,"Setup QPUs arguments: argc={} argv= {} {}", argc, argv[1], argv[2]);
    std::string info_path(argv[1]);
    std::string simulator(argv[2]);
    std::string backend;

    json qpu_config_json;
    try {
        if (argc == 4) {
            backend = std::string(argv[3]);
            std::ifstream f(backend);
            qpu_config_json = json::parse(f);
        } else  if (argc < 2)
            std::cerr << "Error, not a QPU configuration was given\n";

        if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
            if (search->second == SimType::Aer) {
                config::QPUConfig<SimType::Aer> qpu_config{qpu_config_json, info_path};
                QPU<SimType::Aer> qpu(qpu_config);
                SPDLOG_LOGGER_DEBUG(loggie::logger,"Turning ON the QPUs with the AER simulator.");
                qpu.turn_ON();
            } else if (search->second == SimType::Munich) {
                config::QPUConfig<SimType::Munich> qpu_config{qpu_config_json, info_path};
                QPU<SimType::Munich> qpu(qpu_config);
                SPDLOG_LOGGER_DEBUG(loggie::logger,"Turning ON the QPUs with the Munich simulator.");
                qpu.turn_ON();
            }  
        } else
            throw std::runtime_error(std::string("No simulator named ") + simulator);
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(loggie::logger, "Failed turning ON the QPUs because: \n\t{}", e.what());
    }
}