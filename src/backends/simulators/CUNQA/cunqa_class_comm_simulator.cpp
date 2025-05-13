
#include "cunqa_class_comm_simulator.hpp"

#include "instructions.hpp"
#include "result_cunqasim.hpp"
#include "executor.hpp"
#include "config/backend_config.hpp"
#include "config/run_config.hpp"
#include "config/backend_config.hpp"
#include "comm/qpu_comm.hpp"
#include "logger.hpp"

#include "utils/types_cunqasim.hpp"
#include "utils/constants.hpp"
#include "utils/json.hpp"


namespace cunqa {
namespace sim {

CunqaCCSimulator::~CunqaCCSimulator() = default;

cunqa::JSON execute(const SimpleBackend& backend, const QuantumTask& quantumtask) const
{
    JSON instructions = quantumtask.circuit;
    JSON run_config = quantumtask.config;
    int shots = run_config.at("shots");
    std::string instruction_name;
    std::vector<int> qubits;
    std::array<std::string, 1> origin_endpoint;
    std::array<std::string, 1> target_endpoint;
    std::vector<double> param;
    CUNQA::Matrix matrix;
    int measurement;
    JSON result;
    std::unordered_map<int, int> counts;

    int n_qubits = backend.config.at("n_qubits");
    Executor executor(n_qubits);
    LOGGER_DEBUG("Cunqa executor configured with {} qubits.", std::to_string(n_qubits));

    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {
        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name");
            qubits = instruction.at("qubits").get<std::vector<int>>();
            switch (CUNQA::INSTRUCTIONS_MAP[instruction_name])
            {
                case CUNQA::MEASURE:
                    measurement = executor.apply_measure(qubits);
                    break;
                case CUNQA::UNITARY:
                    matrix = instruction.at("params").get<CUNQA::Matrix>();
                    executor.apply_unitary(matrix, qubits);
                    break;
                case CUNQA::ID:
                case CUNQA::X:
                case CUNQA::Y:
                case CUNQA::Z:
                case CUNQA::H:
                case CUNQA::CX:
                case CUNQA::CY:
                case CUNQA::CZ:
                case CUNQA::ECR:
                case CUNQA::C_IF_H:
                case CUNQA::C_IF_X:
                case CUNQA::C_IF_Y:
                case CUNQA::C_IF_Z:
                case CUNQA::C_IF_CX:
                case CUNQA::C_IF_CY:
                case CUNQA::C_IF_CZ:
                case CUNQA::C_IF_ECR:
                    executor.apply_gate(instruction_name, qubits);
                    break;
                case CUNQA::RX:
                case CUNQA::RY:
                case CUNQA::RZ:
                case CUNQA::CRX:
                case CUNQA::CRY:
                case CUNQA::CRZ:
                case CUNQA::C_IF_RX:
                case CUNQA::C_IF_RY:
                case CUNQA::C_IF_RZ:
                    param =  instruction.at("params").get<std::vector<double>>();
                    executor->apply_parametric_gate(instruction_name, qubits, param);
                    break;
                case CUNQA::MEASURE_AND_SEND:
                    target_endpoint = instruction.at("qpus").get<std::array<std::string, 1>>();
                    measurement = executor.apply_measure(qubits); 
                    SPDLOG_LOGGER_DEBUG(logger, "Trying to send to {}", target_endpoint[0]);
                    this->comm_component._send(measurement, target_endpoint[0]); 
                    SPDLOG_LOGGER_DEBUG(logger, "Measurement sent to {}", target_endpoint[0]);
                    break;
                case CUNQA::REMOTE_C_IF_H:
                case CUNQA::REMOTE_C_IF_X:
                case CUNQA::REMOTE_C_IF_Y:
                case CUNQA::REMOTE_C_IF_Z:
                case CUNQA::REMOTE_C_IF_CX:
                case CUNQA::REMOTE_C_IF_CY:
                case CUNQA::REMOTE_C_IF_CZ:
                case CUNQA::REMOTE_C_IF_ECR:
                    origin_endpoint = instruction.at("qpus").get<std::array<std::string, 1>>();
                    SPDLOG_LOGGER_DEBUG(logger, "Origin endpoints: {}", origin_endpoint[0]);
                    measurement = this->comm_component._recv(origin_endpoint[0]); 
                    SPDLOG_LOGGER_DEBUG(logger, "Measurement received from {}", origin_endpoint[0]);
                    if (measurement == 1) {
                        this->backend.apply_gate(CUNQA::CORRESPONDENCE_REMOTE_GATE_MAP[instruction_name], qubits);
                    }
                    break;
                case CUNQA::REMOTE_C_IF_RX:
                case CUNQA::REMOTE_C_IF_RY:
                case CUNQA::REMOTE_C_IF_RZ:
                    origin_endpoint = instruction.at("qpus").get<std::array<std::string, 1>>();
                    SPDLOG_LOGGER_DEBUG(logger, "Origin endpoint: {}", origin_endpoint[0]);
                    param = instruction.at("params").get<std::vector<double>>();
                    measurement = this->comm_component._recv(origin_endpoint[0]); 
                    SPDLOG_LOGGER_DEBUG(logger, "Measurement received from {}", origin_endpoint[0]);
                    if (measurement == 1) {
                        this->backend.apply_gate(CUNQA::CORRESPONDENCE_REMOTE_GATE_MAP[instruction_name], qubits);
                    }
                    break;  
                default:
                    SPDLOG_LOGGER_ERROR(logger, "Invalid gate name."); 
                    throw std::runtime_error("Invalid gate name.");
                    break;
            }
        }
        int position = this->backend.get_shot_result();
        counts[position]++;
        this->backend.restart_statevector();
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


} // End namespace sim
} // End namespace cunqa