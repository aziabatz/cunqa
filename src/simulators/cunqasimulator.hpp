#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "config/run_config.hpp"
#include "config/backend_config.hpp"
#include "logger/logger.hpp"
#include "executor.hpp"
#include "result_cunqasim.hpp"

using json = nlohmann::json;


class CunqaSimulator {

public:
    static json execute(json circuit_json, json& noise_model_json, const config::RunConfig& run_config) {
        
        try {
            QuantumCircuit circuit = circuit_json.at("instructions");
            int n_qubits = circuit_json.at("num_qubits");
            json run_config_json(run_config);
            Executor executor(n_qubits); //TODO: This class is instanciated every time backend.execute is called. Better if only instanciated one time? Where?
            SPDLOG_LOGGER_DEBUG(logger, "Cunqa executor ready to run.");

            ResultCunqa result = executor.run(circuit, run_config_json.at("shots"));

            SPDLOG_LOGGER_DEBUG(logger, "executor.run finished.");
            SPDLOG_LOGGER_DEBUG(logger, "Counts: {}", std::string(result.counts.dump()));

            return result.to_json();
        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error executing the circuit in the Cunqa simulator.\n\tTry checking the format of the sent circuit.");
            return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
        }
        return {};
    }
    
    };