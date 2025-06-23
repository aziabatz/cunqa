
#include "circuit_simulator_adapter.hpp"
#include "munich_executors.hpp"
#include "munich_helpers.hpp"

#include <chrono>
#include <optional>

#include "quantum_task.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "utils/helpers/reverse_bitstring.hpp"

namespace {

}


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

JSON CircuitSimulatorAdapter::simulate(std::size_t shots, std::unique_ptr<comm::ClassicalChannel> classical_channel)

{
     LOGGER_DEBUG("Starting dynamic_execution_ on Munich.");
    // Add the classical channel
    if (classical_channel) {
        std::vector<std::string> connect_with = quantum_task.sending_to;
        classical_channel->connect(connect_with);
    }

    std::map<std::string, std::size_t> measurementCounter;
    JSON result_json;
    float time_taken;
    int n_qubits = quantum_task.config.at("num_qubits");
    const std::size_t nQubits = quantum_task.config.at("num_qubits").get<std::size_t>();
    std::vector<JSON> instructions = quantum_task.circuit;
    JSON run_config = quantum_task.config;
    int shots = quantum_task.config.at("shots").get<int>();
    std::vector<std::uint64_t> qubits;
    std::vector<std::uint64_t> registers;
    //qc::ClassicalRegister clreg; 
    std::vector<std::uint64_t> clreg;
    std::vector<std::uint64_t> conditional_reg;
    std::vector<std::string> endpoint;

    std::unique_ptr<CUNQAQuantumComputation> cqc = std::make_unique<CUNQAQuantumComputation>(quantum_task);
    CUNQACircuitSimulator CUNQAcircsim(std::move(cqc));


    std::map<std::size_t, bool> classicValues;
    std::map<std::size_t, bool> classicRegister;

    LOGGER_DEBUG("Munich variables ready.");

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {

        CUNQAcircsim.CUNQAinitializeSimulation(nQubits);

        for (auto& instruction : instructions) {
            qubits = instruction.at("qubits").get<std::vector<std::uint64_t>>();

            switch (cunqa::constants::INSTRUCTIONS_MAP.at(instruction.at("name")))
            {
                case cunqa::constants::MEASURE:
                    auto clreg = instruction.at("clreg").get<std::vector<std::uint64_t>>();
                    char char_measurement = CUNQAcircsim.CUNQAmeasure(qubits[0]);
                    classicValues[qubits[0]] = (char_measurement == '1');
                    if (!clreg.empty()) {
                        classicRegister[clreg[0]] = (char_measurement == '1');
                    }
                    break;
                case cunqa::constants::X:
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::Y:
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::Z:
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::H:
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::SX:
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::SX);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::RX:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::RY:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::RZ:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CX:
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                   if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CY:
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CZ:
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CRX:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RX, params);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CRY:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RY, params);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CRZ:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RZ, params);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::ECR:
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::ECR);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::CECR:
                    auto p_control = std::make_unique<qc::Control>(qubits[0]);
                    auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::ECR);
                    if (!instruction.contains("conditional_reg")) {
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                        }
                    }
                    break;
                case cunqa::constants::C_IF_H:
                case cunqa::constants::C_IF_X:
                case cunqa::constants::C_IF_Y:
                case cunqa::constants::C_IF_Z:
                case cunqa::constants::C_IF_ECR:
                case cunqa::constants::C_IF_RX:
                case cunqa::constants::C_IF_RY:
                case cunqa::constants::C_IF_RZ:
                    //TODO: Look how Munich natively applies C_IFs operations
                    /* clreg = std::make_pair(conditional_reg[0], 1);
                    auto std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::X);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op); */
                    break;
                case cunqa::constants::MEASURE_AND_SEND:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    char char_measurement = CUNQAcircsim.CUNQAmeasure(qubits[0]);
                    int measurement = char_measurement - '0';
                    classical_channel->send_measure(measurement, endpoint[0]); 
                    break;
                case constants::RECV:
                    
                
                case cunqa::constants::REMOTE_C_IF_H:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_X:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Y:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Z:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RX:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RY:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RZ:
                    auto params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CX:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto p_control = std::make_unique<qc::Control>(qubits[0]);
                        auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CY:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto p_control = std::make_unique<qc::Control>(qubits[0]);
                        auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CZ:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto p_control = std::make_unique<qc::Control>(qubits[0]);
                        auto std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_ECR:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    int measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        auto std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::ECR);
                        CUNQAcircsim.CUNQAapplyOperationToState(std_op);
                    }
                    break;
                default:
                    std::cerr << "Instruction not suported!" << "\n";
            } // End switch
        } // End one shot

        std::string resultString(nQubits, '0');

        // result is a map from the cbit index to the Boolean value
        for (const auto& [bitIndex, value] : classicValues) {
            resultString[nQubits - bitIndex - 1] = value ? '1' : '0';
        }
        measurementCounter[resultString]++;

        classicRegister.clear();
    } // End all shots
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    time_taken = duration.count();
    
    reverse_bitstring_keys_json(measurementCounter);
    result_json = {
        {"counts", measurementCounter},
        {"time_taken", time_taken}
    }; 
    return result_json;
}

} // End of sim namespace
} // End of cunqa namespace