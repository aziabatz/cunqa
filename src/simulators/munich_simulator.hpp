#pragma once

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"
#include "logger/logger.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class MunichSimulator {
public:
    static json execute(json circuit_json, json noise_model_json, const config::RunConfig& run_config) 
    {
        try {
            noise_model_json = noise_model_json;
            SPDLOG_LOGGER_DEBUG(logger, "Noise JSON: {}", noise_model_json.dump(4));

            std::string circuit(circuit_json["circuit"]);
            SPDLOG_LOGGER_DEBUG(logger, "Circuit JSON: {}", circuit);
            auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

            json result_json;
            SPDLOG_LOGGER_DEBUG(logger, "Noise JSON: {}", noise_model_json.dump(4));

            if (!noise_model_json.empty()){
                const ApproximationInfo approx_info{noise_model_json["step_fidelity"], noise_model_json["approx_steps"], ApproximationInfo::FidelityDriven};
                StochasticNoiseSimulator sim(std::move(mqt_circuit), approx_info, run_config.seed, "APD", noise_model_json["noise_prob"],
                                                noise_model_json["noise_prob_t1"], noise_model_json["noise_prob_multi"]);
                auto result = sim.simulate(1024);
                result_json = json(result);
            } else {
                CircuitSimulator sim(std::move(mqt_circuit));
                auto result = sim.simulate(1024);
                result_json = json(result);
            }        

            SPDLOG_LOGGER_DEBUG(logger, "Results: {}", result_json.dump(4));
            return result_json;
        } catch (const std::exception& e) {
            // TODO: specify the circuit format in the docs.
            SPDLOG_LOGGER_ERROR(logger, "Error executing the circuit in the Munich simulator.\nTry checking the format of the circuit sent and/or of the noise model.");
            return {"ERROR", e.what()};
        }
        return {};
    }
};