
#include "circuit_simulator_adapter.hpp"
#include "munich_executors.hpp"
#include "munich_helpers.hpp"

#include <chrono>
#include <optional>

#include "quantum_task.hpp"
#include "backends/simulators/simulator_strategy.hpp"

std::map<std::string, std::size_t> CircuitSimulatorAdapter::simulate(std::size_t shots)
{
    LOGGER_DEBUG("Munich distributed execution");
    // Set the classical channel
    std::vector<std::string> connect_with = quantum_task.sending_to;
    this->classical_channel->connect(connect_with);

    std::map<std::string, std::size_t> measurementCounter;
    JSON result_json;
    float time_taken;
    int n_qubits = quantum_task.config.at("num_qubits").get<int>();
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

    std::unique_ptr<QuantumComputationAdapter> cqc = std::make_unique<QuantumComputationAdapter>(quantum_task);
    CircuitSimulatorAdapter CCcircsim(std::move(cqc));

    const std::size_t nQubits = quantum_task.config.at("num_qubits").get<std::size_t>();

    CCcircsim.initializeSimulationAdapter(nQubits);

    std::map<std::size_t, bool> classicValues;

    for (int i = 0; i < shots; i++) {
        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name");
            qubits = instruction.at("qubits").get<std::vector<std::uint64_t>>();

            switch (cunqa::constants::INSTRUCTIONS_MAP.at(instruction_name))
            {
                case cunqa::constants::MEASURE:
                    char_measurement = CCcircsim.measureAdapter(qubits[0]);
                    classicValues[qubits[0]] = (char_measurement == '1');
                    break;
                case cunqa::constants::X:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::Y:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::Z:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::H:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::SX:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::SX);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CX:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CY:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CZ:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CRX:
                    params = instruction.at("params").get<std::vector<double>>();
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RX, params);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CRY:
                    params = instruction.at("params").get<std::vector<double>>();
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RY, params);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CRZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::RZ, params);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::ECR:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::ECR);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::CECR:
                    pControl = std::make_unique<qc::Control>(qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::ECR);
                    CCcircsim.applyOperationToStateAdapter(std_op);
                    break;
                case cunqa::constants::C_IF_H:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::H);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::C_IF_X:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::X);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::C_IF_Y:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::Y);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::C_IF_Z:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::Z);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::C_IF_RX:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RX, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::C_IF_RY:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RY, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::C_IF_RZ:
                    clreg = std::make_pair(qubits[0], qubits[0]);
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RZ, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std::move(std_op), clreg);
                    CCcircsim.applyOperationToStateAdapter(c_op);
                    break;
                case cunqa::constants::MEASURE_AND_SEND:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    char_measurement = CCcircsim.measureAdapter(qubits[0]);
                    measurement = char_measurement - '0';
                    LOGGER_DEBUG("CHAR_MEASUREMENT: {}, MEASUREMENT: {}", char_measurement, std::to_string(measurement));
                    this->classical_channel->send_measure(measurement, endpoint[0]); 
                    break;
                case cunqa::constants::REMOTE_C_IF_H:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_X:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Y:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Z:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CX:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CY:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CZ:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_ECR:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = this->classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::ECR);
                        CCcircsim.applyOperationToStateAdapter(std_op);
                    }
                    break;
                default:
                    std::cerr << "Instruction not suported!" << "\n";
            } // End switch
        } // End one shot

        std::string resultString(n_qubits, '0');
        
        // result is a map from the cbit index to the Boolean value
        for (const auto& [bitIndex, value] : classicValues) {
        resultString[n_qubits - bitIndex - 1] = value ? '1' : '0';
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

} // End of sim namespace
} // End of cunqa namespace