#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include "logger.hpp"
#include "munich_simple_simulator.hpp"

namespace cunqa {
namespace sim {

JSON MunichSimpleSimulator::execute(const SimpleBackend& backend, const QuantumTask& quantum_task) const
{
    try {

        // TODO: Change the format with the free functions 
        std::string circuit(quantum_task.circuit);
        LOGGER_DEBUG("Circuit cunqa::JSON: {}", circuit);
        auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

        JSON result_json;
        JSON noise_model_json = backend.config.noise_model;
        float time_taken;
        LOGGER_DEBUG("Noise cunqa::JSON: {}", noise_model_json.dump(4));

        if (!noise_model_json.empty()){
            const ApproximationInfo approx_info{noise_model_json["step_fidelity"], noise_model_json["approx_steps"], ApproximationInfo::FidelityDriven};
                StochasticNoiseSimulator sim(std::move(mqt_circuit), approx_info, quantum_task.config["seed"], "APD", noise_model_json["noise_prob"],
                                            noise_model_json["noise_prob_t1"], noise_model_json["noise_prob_multi"]);
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = sim.simulate(quantum_task.config["shots"]);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            time_taken = duration.count();
            
            if (!result.empty())
                return {{"counts", JSON(result)}, {"time_taken", time_taken}};
            throw std::runtime_error("QASM format is not correct."); 
        } else {
            CircuitSimulator sim(std::move(mqt_circuit));
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = sim.simulate(quantum_task.config["shots"]);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            time_taken = duration.count();
            
            if (!result.empty())
                return {{"counts", JSON(result)}, {"time_taken", time_taken}};
            throw std::runtime_error("QASM format is not correct."); 
        }        
    } catch (const std::exception& e) {
        // TODO: specify the circuit format in the docs.
        LOGGER_ERROR("Error executing the circuit in the Munich simulator.\nTry checking the format of the circuit sent and/or of the noise model.");
        return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
    }
    return {};
}

} // End of sim namespace
} // End of cunqa namespace


