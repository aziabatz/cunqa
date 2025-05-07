#pragma once

#include <vector>
#include <optional>

#include "instructions.hpp"
#include "result_cunqasim.hpp"
#include "executor.hpp"

#include "config/backend_config.hpp"
#include "config/run_config.hpp"
#include "config/backend_config.hpp"
#include "comm/qpu_comm.hpp"
#include "utils/types_cunqasim.hpp"
#include "utils/constants.hpp"
#include "utils/json.hpp"
#include "logger.hpp"

#include "simulator.hpp"

class CunqaSimulator {

public:
    std::unique_ptr<Executor> executor;

    void configure_simulator(cunqa::JSON& backend_config)
    {
        int n_qubits = backend_config.at("n_qubits");
        this->executor = std::make_unique<Executor>(n_qubits);
        LOGGER_DEBUG("Cunqa executor configured with {} qubits.", std::to_string(n_qubits));
    }

    //Offloading execution
    cunqa::JSON execute(cunqa::JSON circuit_json, cunqa::JSON& noise_model_json, const config::RunConfig& run_config) {
        LOGGER_ERROR("Error. Offloading execution is not available with Cunqa simulator. ");
        return {};
    }
    

    //Dynamic execution
    inline int _apply_measure(std::vector<int>& qubits)
    {
        try {
            return this->executor->apply_measure(qubits); 
        } catch (const std::exception& e) {
            LOGGER_ERROR("Error executing a measure in the Cunqa simulator.");
            return -1;
        }

    }

    inline void _apply_unitary(CUNQA::Matrix& matrix, std::vector<int>& qubits)
    {
        this->executor->apply_unitary(matrix, qubits);
    }

    inline void _apply_gate(std::string& gate_name, std::vector<int>& qubits, std::vector<double>& param)
    {
        try {

            switch (instructions_map[gate_name])
            {
                case id:
                case x:
                case y:
                case z:
                case h:
                case cx:
                case cy:
                case cz:
                case ecr:
                case c_if_h:
                case c_if_x:
                case c_if_y:
                case c_if_z:
                case c_if_cx:
                case c_if_cy:
                case c_if_cz:
                case c_if_ecr:
                    this->executor->apply_gate(gate_name, qubits);
                    break;
                case rx:
                case ry:
                case rz:
                case crx:
                case cry:
                case crz:
                case c_if_rx:
                case c_if_ry:
                case c_if_rz:
                    this->executor->apply_parametric_gate(gate_name, qubits, param);
                    break;
                default:
                    std::cout << "Error. Invalid gate name" << "\n";
                    break;
            }

        } catch (const std::exception& e) {
            LOGGER_ERROR("Error executing a gate in the Cunqa simulator.");
        }
    }

    inline int _get_statevector_nonzero_position()
    {
        try {
            return this->executor->get_nonzero_position();
        } catch (const std::exception& e) {
            LOGGER_ERROR("Error getting the statevector non-zero position. Check that all qubits are measured.");
            return -1;
        }
    }

    inline void _reinitialize_statevector()
    {
        try {
            this->executor->restart_statevector();
        } catch (const std::exception& e) {
            LOGGER_ERROR("Error restarting the statevector.");
        }
    }
    
};