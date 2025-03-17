#pragma once

#include "simulators/circuit_executor.hpp"
#include "framework/json.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"
#include "logger/logger.hpp"
#include <string>
#include "config/run_config.hpp"
#include "config/backend_config.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std::literals;
using namespace AER;
using namespace config;


class AerSimulator {

public:
    static json execute(json circuit_json, json& noise_model_json, const config::RunConfig& run_config) {
        
        try {
            //TODO: Maybe improve them to send several circuits at once
            Circuit circuit(circuit_json.at("instructions"));
            std::vector<std::shared_ptr<Circuit>> circuits;
            circuits.push_back(std::make_shared<Circuit>(circuit));

            json run_config_json(run_config);
            run_config_json["seed_simulator"] = run_config.seed;
            Config aer_config(run_config_json);
            
            Noise::NoiseModel noise_model(noise_model_json);
            Result result = controller_execute<Controller>(circuits, noise_model, aer_config);
            return result.to_json();
        } catch (const std::exception& e) {
            // TODO: specify the circuit format in the docs.
            SPDLOG_LOGGER_ERROR(logger, "Error executing the circuit in the AER simulator.\n\tTry checking the format of the circuit sent and/or of the noise model.");
            return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
        }
        return {};
    }

};