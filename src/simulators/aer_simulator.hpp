#pragma once

#include "simulators/circuit_executor.hpp"
#include "framework/json.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"
#include <string>
#include "config/run_config.hpp"
#include <nlohmann/json.hpp>

#include "utils/logger.hpp"
//#include <utils/fakeqmio.hpp>


using json = nlohmann::json;
using namespace std::literals;
using namespace AER;
using namespace config;


class AerSimulator {
public:
    
    static json execute(json circuit_json, json noise_model_json, const config::RunConfig& run_config) {
        
        //TODO: Maybe improve them to send several circuits at once
        Circuit circuit(circuit_json);
        std::vector<std::shared_ptr<Circuit>> circuits;
        circuits.push_back(std::make_shared<Circuit>(circuit));

        json run_config_json(run_config);
        Config aer_default(run_config_json);
    
        SPDLOG_LOGGER_ERROR(qpu::logger, "json de ruido es: {}", noise_model_json.dump(4).substr(0, 500));

        Noise::NoiseModel noise_model(noise_model_json[0]);

        Result result = controller_execute<Controller>(circuits, noise_model, aer_default);

        return result.to_json();
    }

};