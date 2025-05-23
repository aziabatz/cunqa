#include "munich_classical_comm_simulator.hpp"

#include <chrono>
#include <optional>

#include "utils/constants.hpp"
#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include "quantum_task.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "munich_helpers.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {


JSON MunichCCSimulator::execute(const ClassicalCommBackend& backend, const QuantumTask& circuit)
{
    LOGGER_DEBUG("We are in the execute() method.");
    if (!circuit.is_distributed) {
        return this->usual_execution_(backend, circuit);
    } else {
        return this->distributed_execution_(backend, circuit);
    }   

} 


std::string MunichCCSimulator::_get_communication_endpoint()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}


// Free functions
JSON MunichCCSimulator::usual_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    try {
        // TODO: Change the format with the free functions 
        std::string circuit = quantum_task_to_Munich(quantum_task);
        LOGGER_DEBUG("OpenQASM circuit: {}", circuit);
        auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

        JSON result_json;
        JSON noise_model_json = backend.config.noise_model;
        float time_taken;

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
        LOGGER_ERROR("Error executing the circuit in the Munich simulator.");
        return {{"ERROR", std::string(e.what()) + ". Try checking the format of the circuit sent and/or of the noise model."}};
    }
    return {};
}

JSON MunichCCSimulator::distributed_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("Munich distributed execution");
    // Set the classical channel
    std::vector<std::string> connect_with = quantum_task.sending_to;
    this->classical_channel->set_classical_connections(connect_with);

    std::map<std::string, std::size_t> measurementCounter;
    JSON result_json;
    float time_taken;
    const std::size_t nQubits = quantum_task.config.at("num_qubits").get<std::size_t>();
    std::vector<JSON> instructions = quantum_task.circuit;
    JSON run_config = quantum_task.config;
    int shots = quantum_task.config.at("shots").get<int>();
    std::string instruction_name;
    std::vector<int> clbits;
    std::vector<std::uint64_t> qubits;
    qc::ClassicalRegister clreg; 
    std::vector<std::string> endpoint;
    std::vector<double> params;
    int measurement;
    char char_measurement;
    std::unique_ptr<qc::Operation> std_op;
    std::unique_ptr<qc::Operation> c_op;
    std::unique_ptr<qc::Control> pControl;

    std::unique_ptr<ClassicalCommQuantumComputation> cqc = std::make_unique<ClassicalCommQuantumComputation>(quantum_task);
    ClassicalCommCircuitSimulator CCcircsim(std::move(cqc));


    std::map<std::size_t, bool> classicValues;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {

        CCcircsim.CCinitializeSimulation(nQubits);

        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name");
            qubits = instruction.at("qubits").get<std::vector<std::uint64_t>>();

            switch (cunqa::constants::INSTRUCTIONS_MAP.at(instruction_name))
            {
                case cunqa::constants::MEASURE:
                    char_measurement = CCcircsim.CCmeasure(qubits[0]);
                    classicValues[qubits[0]] = (char_measurement == '1');
                    break;
                case cunqa::constants::X:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::Y:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::Z:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::H:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::SX:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::SX);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CX:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CY:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CZ:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CRX:
                    params = instruction.at("params").get<std::vector<double>>();
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RX, params);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CRY:
                    params = instruction.at("params").get<std::vector<double>>();
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RY, params);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CRZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RZ, params);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::ECR:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::ECR);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::CECR:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::ECR);
                    CCcircsim.CCapplyOperationToState(std_op);
                    break;
                case cunqa::constants::C_IF_H:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::H);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_X:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::X);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_Y:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::Y);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_Z:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::Z);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_RX:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RX, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_RY:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RY, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_RZ:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RZ, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::MEASURE_AND_SEND:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    char_measurement = CCcircsim.CCmeasure(qubits[0]);
                    measurement = char_measurement - '0';
                    this->classical_channel->send_measure(measurement, endpoint[0]); 
                    break;
                case cunqa::constants::REMOTE_C_IF_H:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_X:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Y:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Z:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CX:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CY:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CZ:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_ECR:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::ECR);
                        CCcircsim.CCapplyOperationToState(std_op);
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
    } // End all shots
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    time_taken = duration.count();
    
    result_json = {
        {"counts", measurementCounter},
        {"time_taken", time_taken}
    }; 
    return result_json;
}



} // End namespace sim
} // End namespace cunqa

