
#include "circuit_simulator_adapter.hpp"
#include "munich_helpers.hpp"

#include <chrono>

#include "quantum_task.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "utils/helpers/reverse_bitstring.hpp"

#include "logger.hpp"

using namespace cunqa;

/*
template <class BackendType>
inline JSON usual_execution_(const BackendType& backend, const QuantumTask& quantum_task)
{
    try {
        // TODO: Change the format with the free functions 
        std::string circuit = quantum_task_to_Munich(quantum_task);
        LOGGER_DEBUG("OpenQASM circuit: {}", circuit);
        auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

        JSON result_json;
        JSON noise_model_json = backend.config.noise_model;
        float time_taken;
        int n_qubits = quantum_task.config.at("num_qubits");

        if (!noise_model_json.empty()){
            LOGGER_DEBUG("Noise model execution");
            const ApproximationInfo approx_info{noise_model_json["step_fidelity"], noise_model_json["approx_steps"], ApproximationInfo::FidelityDriven};
                StochasticNoiseSimulator sim(std::move(mqt_circuit), approx_info, quantum_task.config["seed"], "APD", noise_model_json["noise_prob"],
                                            noise_model_json["noise_prob_t1"], noise_model_json["noise_prob_multi"]);
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = sim.simulate(quantum_task.config["shots"]);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            time_taken = duration.count();
            
            if (!result.empty()) {
                LOGGER_DEBUG("Result non empty");
                reverse_bitstring_keys_json(result);
                return {{"counts", result}, {"time_taken", time_taken}};
            }
            throw std::runtime_error("QASM format is not correct."); 
        } else {
            LOGGER_DEBUG("Noiseless execution");
            CircuitSimulator sim(std::move(mqt_circuit));
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = sim.simulate(quantum_task.config["shots"]);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            time_taken = duration.count();
            
            if (!result.empty()) {
                LOGGER_DEBUG("Result non empty");
                reverse_bitstring_keys_json(result);
                return {{"counts", result}, {"time_taken", time_taken}};
            }
            throw std::runtime_error("QASM format is not correct."); 
        }        
    } catch (const std::exception& e) {
        // TODO: specify the circuit format in the docs.
        LOGGER_ERROR("Error executing the circuit in the Munich simulator.");
        return {{"ERROR", std::string(e.what()) + ". Try checking the format of the circuit sent and/or of the noise model."}};
    }
    return {};
}


*/

namespace cunqa {
namespace sim {

JSON CircuitSimulatorAdapter::simulate(std::size_t shots, comm::ClassicalChannel* classical_channel)
{
    // TODO
    auto p_qca = static_cast<QuantumComputationAdapter*>(qc.get());
    
    std::map<std::string, std::size_t> meas_counter;

    if (classical_channel && p_qca->quantum_tasks.size() == 1) {
        std::vector<std::string> connect_with = p_qca->quantum_tasks[0].sending_to;
        classical_channel->connect(connect_with);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {  
        meas_counter[execute_shot_(classical_channel, p_qca->quantum_tasks)]++;
    } // End all shots
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    float time_taken = duration.count();
    
    reverse_bitstring_keys_json(meas_counter);
    JSON result_json = {
        {"counts", meas_counter},
        {"time_taken", time_taken}
    };
    return result_json;
}

void CircuitSimulatorAdapter::apply_gate_(const JSON& instruction, std::unique_ptr<qc::StandardOperation>&& std_op, std::map<std::size_t, bool>& classic_reg, std::map<std::size_t, bool>& r_classic_reg)
{ 
    if (instruction.contains("conditional_reg")) {
        auto conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
        if (classic_reg[conditional_reg[0]]) {
            applyOperationToStateAdapter(std::move(std_op));
        }
    } else if (instruction.contains("remote_conditional_reg")) {
        auto conditional_reg = instruction.at("remote_conditional_reg").get<std::vector<std::uint64_t>>();
        if (r_classic_reg[conditional_reg[0]]) {
            applyOperationToStateAdapter(std::move(std_op));
        }
    } else
        applyOperationToStateAdapter(std::move(std_op));
}

std::string CircuitSimulatorAdapter::execute_shot_(comm::ClassicalChannel* classical_channel, const std::vector<QuantumTask>& quantum_tasks)
{
    std::vector<JSON::const_iterator> its;
    std::vector<JSON::const_iterator> ends;
    std::unordered_map<std::string, bool> blocked;
    std::vector<int> zero_qubit;
    int n_qubits = 0;
    
    for (auto &quantum_task : quantum_tasks) {
        zero_qubit.push_back(n_qubits);
        its.push_back(quantum_task.circuit.begin());
        ends.push_back(quantum_task.circuit.end());
        n_qubits += quantum_task.config.at("num_qubits").get<int>();
        blocked[quantum_task.id] = false;
    }

    initializeSimulationAdapter(n_qubits);

    std::vector<int> qubits;
    std::map<std::size_t, bool> classic_values;
    std::map<std::size_t, bool> classic_reg;
    std::map<std::size_t, bool> r_classic_reg;

    bool ended = false;
    while (!ended) {
        ended = true;
        for (size_t i = 0; i < its.size(); ++i) {
            if(its[i] == ends[i] || blocked[quantum_tasks[i].id])
                continue;

            auto& instruction = *its[i];
            qubits = instruction.at("qubits").get<std::vector<int>>();
            switch (constants::INSTRUCTIONS_MAP.at(instruction.at("name")))
            {
                case constants::MEASURE:
                {
                    auto clreg = instruction.at("clreg").get<std::vector<std::uint64_t>>();
                    char char_measurement = measureAdapter(qubits[0] + zero_qubit[i]);
                    classic_values[qubits[0] + zero_qubit[i]] = (char_measurement == '1');
                    if (!clreg.empty()) {
                        classic_reg[clreg[0]] = (char_measurement == '1');
                    }
                    break;
                }
                case constants::X:
                {
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::X);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::Y:
                {
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::Y);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::Z:
                {
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::Z);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::H:
                {
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::H);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::SX:
                {
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::SX);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::RX:
                {
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::RX, params);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::RY:
                {
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::RY, params);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::RZ:
                {
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::RZ, params);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CX:
                {
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::X);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CY:
                {
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::Y);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CZ:
                {
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::Z);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CRX:
                {
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::RX, params);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CRY:
                {
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::RY, params);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CRZ:
                {
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::RZ, params);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::ECR:
                {
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0] + zero_qubit[i], qc::OpType::ECR);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::CECR:
                {
                    auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::ECR);
                    apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    break;
                }
                case constants::C_IF_H:
                case constants::C_IF_X:
                case constants::C_IF_Y:
                case constants::C_IF_Z:
                case constants::C_IF_ECR:
                case constants::C_IF_RX:
                case constants::C_IF_RY:
                case constants::C_IF_RZ:
                    //TODO: Look how Munich natively applies C_IFs operations
                    /* clreg = std::make_pair(conditional_reg[0], 1);
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[1] + zero_qubit[i], qc::OpType::X);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, clreg);
                    CCcircsim.CCapplyOperationToState(c_op); */
                    break;
                case constants::MEASURE_AND_SEND:
                {
                    auto endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    char char_measurement = measureAdapter(qubits[0] + zero_qubit[i]);
                    int measurement = char_measurement - '0';
                    classical_channel->send_measure(measurement, endpoint[0]); 
                    break;
                }
                case constants::RECV:
                {
                    auto endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    auto conditional_reg = instruction.at("remote_conditional_reg").get<std::vector<std::uint64_t>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]);
                    r_classic_reg[conditional_reg[0]] = (measurement == 1);
                    break;
                }
                case constants::QSEND:
                {
                    //auto p_control = std::make_unique<qc::Control>(qubits[0] + zero_qubit[i]);
                    //auto std_op = std::make_unique<qc::StandardOperation>(*p_control, qubits[1] + zero_qubit[i], qc::OpType::X);
                    //apply_gate_(instruction, std::move(std_op), classic_reg, r_classic_reg);
                    const auto example_qubit = qubits[0] + zero_qubit[i];
                    LOGGER_DEBUG("Sending qubit: {} to QPU {}", example_qubit, 0);
                    break;
                }
                case constants::QRECV:
                {
                    const auto example_qubit = qubits[0] + zero_qubit[i];
                    LOGGER_DEBUG("Receiving qubit: {} from QPU {}", example_qubit, 0);
                    break;
                }
                default:
                    std::cerr << "Instruction not suported!" << "\n";
            } // End switch
            ++its[i];
            ended = its[i] != ends[i] ? false : ended;
        }
    } // End one shot

    std::string resultString(n_qubits, '0');

    // result is a map from the cbit index to the Boolean value
    for (const auto& [bitIndex, value] : classic_values) {
        resultString[n_qubits - bitIndex - 1] = value ? '1' : '0';
    }
    return resultString;
}

} // End of sim namespace
} // End of cunqa namespace