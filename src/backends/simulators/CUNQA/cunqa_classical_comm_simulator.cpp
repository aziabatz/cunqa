
#include "cunqa_classical_comm_simulator.hpp"

#include "src/instructions.hpp"
#include "src/result_cunqasim.hpp"
#include "src/executor.hpp"
#include "src/utils/types_cunqasim.hpp"

#include "utils/json.hpp"
#include "utils/constants.hpp"
#include "logger.hpp"

namespace cunqa {
namespace sim {

CunqaCCSimulator::~CunqaCCSimulator() = default;

JSON CunqaCCSimulator::execute(const SimpleBackend& backend, const QuantumTask& quantumtask)
{
    std::vector<JSON> instructions = quantumtask.circuit;
    JSON run_config = quantumtask.config;
    int shots = run_config.at("shots");
    std::string instruction_name;
    std::vector<int> qubits;
    std::vector<std::string> origin_endpoint;
    std::vector<std::string> target_endpoint;
    std::vector<double> param;
    Matrix matrix;
    int measurement;
    JSON result;
    std::unordered_map<int, int> counts;

    int n_qubits = quantumtask.config.at("num_qubits");
    if (executor != nullptr && (n_qubits == executor->n_qubits)) {
        LOGGER_DEBUG("Cunqa executor already configured with {} qubits.", std::to_string(n_qubits));
    } else {
        executor = std::make_unique<Executor>(n_qubits);
        LOGGER_DEBUG("Executor created with {} qubits.", std::to_string(n_qubits));
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {
        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name").get<std::string>();
            qubits = instruction.at("qubits").get<std::vector<int>>();
            LOGGER_DEBUG("Instruction name and qubits well read.");
            switch (constants::INSTRUCTIONS_MAP.at(instruction_name))
            {
                case constants::MEASURE:
                    measurement = executor->apply_measure(qubits);
                    break;
                case constants::UNITARY:
                    matrix = instruction.at("params").get<Matrix>();
                    executor->apply_unitary(matrix, qubits);
                    break;
                case constants::ID:
                case constants::X:
                case constants::Y:
                case constants::Z:
                case constants::H:
                case constants::SX:
                case constants::CX:
                case constants::CY:
                case constants::CZ:
                case constants::ECR:
                case constants::C_IF_H:
                case constants::C_IF_X:
                case constants::C_IF_Y:
                case constants::C_IF_Z:
                case constants::C_IF_CX:
                case constants::C_IF_CY:
                case constants::C_IF_CZ:
                case constants::C_IF_ECR:
                    executor->apply_gate(instruction_name, qubits);
                    break;
                case constants::RX:
                case constants::RY:
                case constants::RZ:
                case constants::CRX:
                case constants::CRY:
                case constants::CRZ:
                case constants::C_IF_RX:
                case constants::C_IF_RY:
                case constants::C_IF_RZ:
                    param =  instruction.at("params").get<std::vector<double>>();
                    executor->apply_parametric_gate(instruction_name, qubits, param);
                    break;
                case constants::MEASURE_AND_SEND:
                    target_endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = executor->apply_measure(qubits); 
                    SPDLOG_LOGGER_DEBUG(logger, "Trying to send to {}", target_endpoint[0]);
                    classical_channel->send_measure(measurement, target_endpoint[0]); 
                    SPDLOG_LOGGER_DEBUG(logger, "Measurement sent to {}", target_endpoint[0]);
                    break;
                case constants::REMOTE_C_IF_H:
                case constants::REMOTE_C_IF_X:
                case constants::REMOTE_C_IF_Y:
                case constants::REMOTE_C_IF_Z:
                case constants::REMOTE_C_IF_CX:
                case constants::REMOTE_C_IF_CY:
                case constants::REMOTE_C_IF_CZ:
                case constants::REMOTE_C_IF_ECR:
                    origin_endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    SPDLOG_LOGGER_DEBUG(logger, "Origin endpoint: {}", origin_endpoint[0]);
                    measurement = classical_channel->recv_measure(origin_endpoint[0]); 
                    SPDLOG_LOGGER_DEBUG(logger, "Measurement received from {}", origin_endpoint[0]);
                    if (measurement == 1) {
                        executor->apply_gate(constants::CORRESPONDENCE_REMOTE_GATE_MAP.at(instruction_name), qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_RX:
                case constants::REMOTE_C_IF_RY:
                case constants::REMOTE_C_IF_RZ:
                    origin_endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    SPDLOG_LOGGER_DEBUG(logger, "Origin endpoint: {}", origin_endpoint[0]);
                    param = instruction.at("params").get<std::vector<double>>();
                    measurement = classical_channel->recv_measure(origin_endpoint[0]); 
                    SPDLOG_LOGGER_DEBUG(logger, "Measurement received from {}", origin_endpoint[0]);
                    if (measurement == 1) {
                        executor->apply_gate(constants::CORRESPONDENCE_REMOTE_GATE_MAP.at(instruction_name), qubits);
                    }
                    break;  
                default:
                    SPDLOG_LOGGER_ERROR(logger, "Invalid gate name."); 
                    throw std::runtime_error("Invalid gate name.");
                    break;
            }
        }
        int position = executor->get_nonzero_position();
        counts[position]++;
        executor->restart_statevector();
    }

    auto stop_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);
    double total_time = duration.count();

    result = {
        {"counts", counts},
        {"time_taken", total_time}
    }; 

    counts.clear();

    return result;

}

std::string CunqaCCSimulator::get_communication_endpoint()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}

} // End namespace sim
} // End namespace cunqa