#pragma once


#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include "comm/qpu_comm.hpp"
#include "config/backend_config.hpp"
#include "utils/constants.hpp"
#include "utils/json.hpp"
#include "logger/logger.hpp"

#include "simulator.hpp"

class MunichSimulator {
public:
    int munich_mpi_rank;
    MunichSimulator()
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &munich_mpi_rank);
        LOGGER_DEBUG("munich_mpi_rank: {}", munich_mpi_rank);
    }
    
    void configure_simulator(cunqa::JSON& backend_config)
    {
        LOGGER_DEBUG("No configuration needed for MunichSimulator");
    }

    //Offloading execution
    cunqa::JSON execute(cunqa::JSON circuit_json, cunqa::JSON& noise_model_json,  const config::RunConfig& run_config) 
    {
        try {
            LOGGER_DEBUG("Noise cunqa::JSON: {}", noise_model_json.dump(4));

            std::string circuit(circuit_json.at("instructions"));
            LOGGER_DEBUG("Circuit cunqa::JSON: {}", circuit);
            auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

            cunqa::JSON result_json;
            float time_taken;
            LOGGER_DEBUG("Noise cunqa::JSON: {}", noise_model_json.dump(4));

            if (!noise_model_json.empty()){
                const ApproximationInfo approx_info{noise_model_json["step_fidelity"], noise_model_json["approx_steps"], ApproximationInfo::FidelityDriven};
                StochasticNoiseSimulator sim(std::move(mqt_circuit), approx_info, run_config.seed, "APD", noise_model_json["noise_prob"],
                                                noise_model_json["noise_prob_t1"], noise_model_json["noise_prob_multi"]);
                auto start = std::chrono::high_resolution_clock::now();
                auto result = sim.simulate(run_config.shots);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> duration = end - start;
                time_taken = duration.count();
                !result.empty() ? result_json = cunqa::JSON(result) : throw std::runtime_error("QASM format is not correct.");
            } else {
                CircuitSimulator sim(std::move(mqt_circuit));
                auto start = std::chrono::high_resolution_clock::now();
                auto result = sim.simulate(run_config.shots);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> duration = end - start;
                time_taken = duration.count();
                !result.empty() ? result_json = cunqa::JSON(result) : throw std::runtime_error("QASM format is not correct.");
            }        

            LOGGER_DEBUG("Results: {}", result_json.dump(4));
            return cunqa::JSON({{"counts", result_json}, {"time_taken", time_taken}});
        } catch (const std::exception& e) {
            // TODO: specify the circuit format in the docs.
            LOGGER_ERROR("Error executing the circuit in the Munich simulator.\nTry checking the format of the circuit sent and/or of the noise model.");
            return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
        }
        return {};
    }

    
    //Dynamic execution
    inline int _apply_measure(std::array<int, 3>& qubits)
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
        return -1;
    }
    
    inline void _apply_gate(std::string& gate_name, std::array<int, 3>& qubits, std::vector<double>& param)
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
    }

    inline int _get_statevector_nonzero_position()
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
        return -1;
    }

    inline void _reinitialize_statevector()
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
    }
};