#pragma once

#include <string>
#include <memory>
#include <vector>
#include <complex>
#include "zmq.hpp"

#include "simulators/simulator.hpp"
#include "logger/logger.hpp"
#include "classical_node_utils.hpp"
#include "communication_component.hpp"

#include "logger/logger.hpp"


using json = nlohmann::json;
using CunqaInstructions = std::vector<json>; 
using CunqaStateVector = std::vector<std::complex<double>>;


template <SimType sim_type>
class QPUClassicalNode 
{
public:

    Backend<sim_type> backend;
    CommunicationComponent<sim_type> comm_component;
    
    json result_circuit = {};
    
    QPUClassicalNode(std::string& comm_type, config::QPUConfig<sim_type> qpu_config, int& argc, char *argv[]);

    inline void send_circ_to_execute(json& kernel);
    inline void send_instructions_to_execute(json& kernel);
    inline void clean_circuit_result();

};

template <SimType sim_type>
QPUClassicalNode<sim_type>::QPUClassicalNode(std::string& comm_type, config::QPUConfig<sim_type> qpu_config, int& argc, char *argv[]) : backend(qpu_config.backend_config), comm_component(comm_type, argc, argv)
{
    SPDLOG_LOGGER_DEBUG(logger, "Classical Node correctly instanciated.");
}

template <SimType sim_type>
inline void QPUClassicalNode<sim_type>::send_circ_to_execute(json& kernel)
{
    this->result_circuit = this->backend.run(kernel);
    SPDLOG_LOGGER_DEBUG(logger, "Result is in the classical node.");
}

template <SimType sim_type>
inline void QPUClassicalNode<sim_type>::send_instructions_to_execute(json& kernel)
{
    CunqaInstructions instructions = kernel.at("instructions");
    int n_qubits = kernel.at("num_qubits");
    config::RunConfig run_config(kernel.at("config"));
    json run_config_json(run_config);
    int shots = run_config_json.at("shots");
    std::string instruction_name;
    std::array<int, 3> qubits;
    int measurement;
    std::vector<double> param;
    std::unordered_map<int, int> counts;

    this->backend.initialize_statevector(n_qubits);


    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {
        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name");
            qubits = instruction.at("qubits");
            switch (CUNQA_INSTRUCTIONS_MAP[instruction_name])
            {
                case measure:
                    measurement = this->backend.apply_measure(instruction_name, qubits);
                    break;
                case cunqa_id:
                case cunqa_x:
                case cunqa_y:
                case cunqa_z:
                case cunqa_h:
                case cunqa_cx:
                case cunqa_cy:
                case cunqa_cz:
                case cunqa_ecr:
                case cunqa_c_if_h:
                case cunqa_c_if_x:
                case cunqa_c_if_y:
                case cunqa_c_if_z:
                case cunqa_c_if_cx:
                case cunqa_c_if_cy:
                case cunqa_c_if_cz:
                case cunqa_c_if_ecr:
                    this->backend.apply_gate(instruction_name, qubits);
                    break;
                case cunqa_rx:
                case cunqa_ry:
                case cunqa_rz:
                case cunqa_c_if_rx:
                case cunqa_c_if_ry:
                case cunqa_c_if_rz:
                    param =  instruction.at("params").get<std::vector<double>>();
                    this->backend.apply_gate(instruction_name, qubits, param);
                    break;
                case cunqa_d_c_if_h:
                case cunqa_d_c_if_x:
                case cunqa_d_c_if_y:
                case cunqa_d_c_if_z:
                case cunqa_d_c_if_cx:
                case cunqa_d_c_if_cy:
                case cunqa_d_c_if_cz:
                case cunqa_d_c_if_ecr:
                    if (this->comm_component.comm_type == "mpi") {
                        std::array<int, 2> comm_endp = instruction.at("qpus").get<std::array<int, 2>>();
                        if (comm_endp[0] == this->comm_component.mpi_rank) {
                            measurement = this->backend.apply_measure(instruction_name, qubits);
                            this->comm_component._send(measurement, comm_endp[1]);
                        } else if (comm_endp[1] == this->comm_component.mpi_rank) {
                            measurement = this->comm_component._recv(comm_endp[0]);
                            if (measurement == 1) {
                                this->backend.apply_gate(CORRESPONDENCE_D_GATE_MAP[instruction_name], qubits);
                            }
    
                        } else {
                            SPDLOG_LOGGER_ERROR(logger, "This QPU has no ID {} nor {}, but {}", comm_endp[0], comm_endp[1], this->comm_component.mpi_rank); 
                            throw std::runtime_error("Error with the QPU IDs.");
                        }
                    } else if (this->comm_component.comm_type == "zmq") {
                        std::array<std::string, 2> comm_endp = instruction.at("qpus").get<std::array<std::string, 2>>();
                        if (comm_endp[0] == this->comm_component.zmq_endpoint.value()) {
                            measurement = this->backend.apply_measure(instruction_name, qubits);
                            this->comm_component._send(measurement, comm_endp[1]);

                        } else if (comm_endp[1] == this->comm_component.zmq_endpoint.value()) {
                            measurement = this->comm_component._recv(comm_endp[0]);

                            if (measurement == 1) {
                                this->backend.apply_gate(CORRESPONDENCE_D_GATE_MAP[instruction_name], qubits);
                            }
                        } else {
                            SPDLOG_LOGGER_ERROR(logger, "This QPU has no ID {} nor {}, but {}", comm_endp[0], comm_endp[1], this->comm_component.zmq_endpoint.value()); 
                            throw std::runtime_error("Error with the QPU IDs.");
                        }
                    }
                    break;
                case cunqa_d_c_if_rx:
                case cunqa_d_c_if_ry:
                case cunqa_d_c_if_rz:
                    param = instruction.at("params").get<std::vector<double>>();
                    if (this->comm_component.comm_type == "mpi") {
                        std::array<int, 2> comm_endp = instruction.at("qpus").get<std::array<int, 2>>();
                        if (comm_endp[0] == this->comm_component.mpi_rank) {
                            measurement = this->backend.apply_measure(instruction_name, qubits);
                            this->comm_component._send(measurement, comm_endp[1]);
                        } else if (comm_endp[1] == this->comm_component.mpi_rank) {
                            measurement = this->comm_component._recv(comm_endp[0]);
                            if (measurement == 1) {
                                this->backend.apply_gate(CORRESPONDENCE_D_GATE_MAP[instruction_name], qubits, param);
                            }
    
                        } else {
                            SPDLOG_LOGGER_ERROR(logger, "This QPU has no ID {} nor {}, but {}", comm_endp[0], comm_endp[1], this->comm_component.mpi_rank); 
                            throw std::runtime_error("Error with the QPU IDs.");
                        }
                    } else if (this->comm_component.comm_type == "zmq") {
                        std::array<std::string, 2> comm_endp = instruction.at("qpus").get<std::array<std::string, 2>>();
                        if (comm_endp[0] == this->comm_component.zmq_endpoint.value()) {
                            measurement = this->backend.apply_measure(instruction_name, qubits);
                            this->comm_component._send(measurement, comm_endp[1]);

                        } else if (comm_endp[1] == this->comm_component.zmq_endpoint.value()) {
                            std::string client_id = this->comm_component._client_id_recv();
                            measurement = this->comm_component._recv(comm_endp[0]);

                            if (measurement == 1) {
                                this->backend.apply_gate(CORRESPONDENCE_D_GATE_MAP[instruction_name], qubits, param);
                            }
    
                        } else {
                            SPDLOG_LOGGER_ERROR(logger, "This QPU has no ID {} nor {}, but {}", comm_endp[0], comm_endp[1], this->comm_component.zmq_endpoint.value()); 
                            throw std::runtime_error("Error with the QPU IDs.");
                        }
                    }
                    
                    break;  
                
                default:
                    std::cout << "Error. Invalid gate name" << "\n";
                    break;
            }
        }
        int position = this->backend.get_statevector_status();
        SPDLOG_LOGGER_DEBUG(logger, "Position: {}", position);
        counts[position]++;
        this->backend._restart_statevector();
    }

    auto stop_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);
    double total_time = duration.count();

    this->result_circuit = counts;

}

template <SimType sim_type>
inline void QPUClassicalNode<sim_type>::clean_circuit_result()
{
    this->result_circuit.clear();
}

