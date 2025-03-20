#pragma once

#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

#include "config/run_config.hpp"
#include "config/backend_config.hpp"
#include "comm/qpu_comm.hpp"
#include "logger/logger.hpp"
#include "executor.hpp"
#include "result_cunqasim.hpp"
#include "utils/types_cunqasim.hpp"

using json = nlohmann::json;


class CunqaSimulator {

public:
    static json execute(json circuit_json, json& noise_model_json, const config::RunConfig& run_config, std::optional<ZMQSockets> zmq_sockets) {
        
        try {
            QuantumCircuit circuit = circuit_json.at("instructions");
            int n_qubits = circuit_json.at("num_qubits");
            json run_config_json(run_config);

            #if defined(QPU_MPI) || defined(NO_COMM)
                Executor executor(n_qubits); 
                SPDLOG_LOGGER_DEBUG(logger, "Cunqa executor ready to run.");
                ResultCunqa result = executor.run(circuit, run_config_json.at("shots"));
            #elif defined(QPU_ZMQ)
                Executor executor(n_qubits, 
                    std::move(zmq_sockets->client),
                    std::move(zmq_sockets->server),
                    std::move(zmq_sockets->zmq_endpoint)); 
                SPDLOG_LOGGER_DEBUG(logger, "Cunqa executor ready to run."); 
                ResultCunqa result = executor.run(circuit, run_config_json.at("shots"));
            #endif

            SPDLOG_LOGGER_DEBUG(logger, "executor.run finished.");

            return result.to_json();
        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error executing the circuit in the Cunqa simulator.\n\tTry checking the format of the sent circuit.");
            return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
        }
        return {};
    }
    
};