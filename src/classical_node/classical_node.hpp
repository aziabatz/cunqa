#pragma once

#include <string>
#include <memory>
#include <vector>
#include <complex>
#include "zmq.hpp"

#include "simulators/simulator.hpp"
#include "classical_node_utils.hpp"
#include "communication_component.hpp"

#include "logger/logger.hpp"


using json = nlohmann::json;
using CunqaInstructions = std::vector<json>;


template <SimType sim_type>
class QPUClassicalNode 
{
public:

    Backend<sim_type> backend;
    CommunicationComponent<sim_type> comm_component;
    json endpoint_info = {};
    json result = {};
    
    QPUClassicalNode(std::string& comm_type, config::QPUConfig<sim_type> qpu_config);

    inline void send_circ_to_execute(json& kernel);
    inline void send_instructions_to_execute(json& kernel);
    inline void clean_result();

};

template <SimType sim_type>
QPUClassicalNode<sim_type>::QPUClassicalNode(std::string& comm_type, config::QPUConfig<sim_type> qpu_config) : backend(qpu_config.backend_config), comm_component(comm_type)
{
    SPDLOG_LOGGER_DEBUG(logger, "Classical Node correctly instanciated.");

    std::string endpoint_type;
    std::string endpoint_value;
    if (this->comm_component.comm_type == "mpi") {
        endpoint_type = "mpi";
        endpoint_value = std::to_string(this->comm_component.mpi_rank);
    } else if (this->comm_component.comm_type == "zmq") {
        endpoint_type = "zmq";
        endpoint_value = this->comm_component.zmq_endpoint.value();
    } else {
        endpoint_type = "none";
        endpoint_value = "";
    }

    this->endpoint_info = {
        {endpoint_type, endpoint_value}
    };
}

template <SimType sim_type>
inline void QPUClassicalNode<sim_type>::send_circ_to_execute(json& kernel)
{
    this->result = this->backend.run(kernel);
    SPDLOG_LOGGER_DEBUG(logger, "Result is in the classical node.");
}

template <SimType sim_type>
inline void QPUClassicalNode<sim_type>::send_instructions_to_execute(json& kernel)
{
    CunqaInstructions instructions = kernel.at("instructions");
    config::RunConfig run_config(kernel.at("config"));
    json run_config_json(run_config);
    int shots = run_config_json.at("shots");
    std::string instruction_name;
    std::vector<int> qubits;
    CUNQA::Matrix matrix;
    int measurement;
    std::array<std::string, 1> origin_endpoint;
    std::array<std::string, 1> target_endpoint;
    std::vector<double> param;
    std::unordered_map<int, int> counts;

    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {
        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name");
            qubits = instruction.at("qubits").get<std::vector<int>>();
            switch (CUNQA::INSTRUCTIONS_MAP[instruction_name])
            {
                case CUNQA::MEASURE:
                    measurement = this->backend.apply_measure(qubits);
                    break;
                case CUNQA::UNITARY:
                    matrix = instruction.at("params").get<CUNQA::Matrix>();
                    this->backend.apply_unitary(matrix, qubits);
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
                    this->backend.apply_gate(instruction_name, qubits);
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
                    this->backend.apply_gate(instruction_name, qubits, param);
                    break;
                case CUNQA::MEASURE_AND_SEND:
                    target_endpoint = instruction.at("qpus").get<std::array<std::string, 1>>();
                    measurement = this->backend.apply_measure(qubits); 
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

    this->result = {
        {"counts", counts},
        {"time_taken", total_time}
    }; 

    counts.clear();

}

template <SimType sim_type>
inline void QPUClassicalNode<sim_type>::clean_result()
{
    this->result.clear();
}

