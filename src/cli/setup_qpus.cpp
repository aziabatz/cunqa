//#include "qpu.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
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
    json qpu_config_json;
    if (argc == 4) 
    {
        std::ifstream f(argv[3]);
        qpu_config_json = json::parse(f);
    } 
    else  if (argc < 2)
        std::cerr << "Error, not a QPU configuration was given\n";

    try {
        if(auto search = SIM_NAMES.find(argv[2]); search != SIM_NAMES.end()) {
            if (search->second == SimType::Aer) {
                config::QPUConfig<SimType::Aer> qpu_config{qpu_config_json, argv[1]};
                QPU<SimType::Aer> qpu(qpu_config, argv[3]);
                SPDLOG_LOGGER_DEBUG(loggie::logger,"Turning ON the QPUs with the AER simulator.");
                qpu.turn_ON();
            } else if (search->second == SimType::Munich) {
                config::QPUConfig<SimType::Munich> qpu_config{qpu_config_json, argv[1]};
                QPU<SimType::Munich> qpu(qpu_config, argv[3]);
                SPDLOG_LOGGER_DEBUG(loggie::logger,"Turning ON the QPUs with the Munich simulator.");
                qpu.turn_ON();
            }  
        }
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(loggie::logger, "Failed turning ON the QPUs because: \n\t{}", e.what());
    }
    
}