#pragma once

#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

#include "config/run_config.hpp"
#include "config/backend_config.hpp"
#include "comm/qpu_comm.hpp"
#include "logger/logger.hpp"
#include "instructions.hpp"
#include "result_cunqasim.hpp"
#include "utils/types_cunqasim.hpp"
#include "utils/constants.hpp"

using json = nlohmann::json;



class CunqaSimulator {

public:
    static json execute(json circuit_json, json& noise_model_json, const config::RunConfig& run_config) {
        SPDLOG_LOGGER_ERROR(logger, "Error. Offloading execution is not available with Cunqa simulator. ");
        return {};
    }
        
    
    static CunqaStateVector _apply_gate(std::string& instruction_name, CunqaStateVector& statevector, std::array<int, 3>& qubits, std::vector<double>& param)
    {
        try {
            SPDLOG_LOGGER_DEBUG(logger, "Cunqa gate apply ready.");

            switch (instructions_map[instruction_name])
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
                    statevector = Instruction::apply_instruction(statevector, instruction_name, qubits);
                    break;
                case rx:
                case ry:
                case rz:
                case c_if_rx:
                case c_if_ry:
                case c_if_rz:
                    statevector = Instruction::apply_param_instruction(statevector, instruction_name, qubits, param);
                    break;
                default:
                    std::cout << "Error. Invalid gate name" << "\n";
                    break;
            }

        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error executing a gate in the Cunqa simulator.");
        }

        return statevector;
    }

    static MeasurementOutput _apply_measure(std::string& instruction_name, CunqaStateVector& statevector, std::array<int, 3>& qubits)
        {
            meas_out cunqa_measurement;
            MeasurementOutput measurement;

            //int measurement_value;

            try {
                cunqa_measurement = Instruction::apply_measure(statevector, qubits); 
                //measurement_value = measurement.measure;
                measurement.statevector = cunqa_measurement.statevector;
                measurement.measure = cunqa_measurement.measure;
                return measurement;
                
            } catch (const std::exception& e) {
                SPDLOG_LOGGER_ERROR(logger, "Error executing a measure in the Cunqa simulator.");
                return measurement;
            }
        }

};