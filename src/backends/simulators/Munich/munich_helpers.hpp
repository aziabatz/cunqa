
#include <iostream>
#include <vector>
#include "utils/constants.hpp"
#include "quantum_task.hpp"
#include <nlohmann/json.hpp>

using JSON = nlohmann::json;
namespace cunqa {

class MunichConfig {
    //
};


// Used in the quantum_task_to_Munich function for printing correctly the matrices of custom unitary gates
std::string triple_vector_to_string(const std::vector<std::vector<std::vector<double>>>& data) {
    std::ostringstream oss;
    oss << "[";

    for (size_t i = 0; i < data.size(); ++i) {
        oss << "[";
        for (size_t j = 0; j < data[i].size(); ++j) {
            oss << "[";
            for (size_t k = 0; k < data[i][j].size(); ++k) {
                oss << data[i][j][k];
                if (k != data[i][j].size() - 1) oss << ", ";
            }
            oss << "]";
            if (j != data[i].size() - 1) oss << ", ";
        }
        oss << "]";
        if (i != data.size() - 1) oss << ", ";
    }

    oss << "]";
    return oss.str();
}


// Transfroms a quantum_task, which has config and circuit (these are the instructions) to an OpenQASM2 string
std::string quantum_task_to_Munich(QuantumTask& quantum_task) 
{ 
    JSON instructions = quantum_task.circuit;
    JSON config_json = quantum_task.config;
    std::string qasm_circt = "openqasm 2.0;\ninclude \"qelib1.inc\";\n";

    std::cout << "Empieza la diversión" << "\n";

    // Quantum register declaration
    auto quantum_registers = config_json.at("quantum_registers");
    for (auto& reg : quantum_registers.items()) {
        std::string reg_name = reg.key();
        auto qubits = reg.value();
        qasm_circt += "qreg " + reg_name + "[" + std::to_string(qubits.size()) + "];\n";
    }

    // Classical register declaration
    auto classical_registers = config_json.at("classical_registers");
    for (auto& reg : classical_registers.items()) {
        std::string reg_name = reg.key();
        auto clbits = reg.value();
        qasm_circt += "creg " + reg_name + "[" + std::to_string(clbits.size()) + "];\n";
    }

    // Instruction processing
    for (const auto& instruction : instructions) {
        std::string gate_name = instruction.at("name");
        std::cout << "Sacar el nombre de una instruction rula " << gate_name << "\n";
        auto qubits = instruction.at("qubits");
        std::cout << "Sacar los qubits de una instruction rula " << qubits[0] << "\n";
        std::vector<double> params;
        std::vector<std::vector<std::vector<std::vector<double>>>> matrix;
        

        try {
            switch (cunqa::INSTRUCTIONS_MAP.at(gate_name))
            {   
                // Non-parametric 1 qubit gates
                case cunqa::ID:
                case cunqa::X:
                case cunqa::Y:
                case cunqa::Z:
                case cunqa::H:
                case cunqa::SX:
                //case cunqa::S:
                //case cunqa::SDG:
                //case cunqa::SXDG:
                //case cunqa::T:
                //case cunqa::TDG:
                    qasm_circt += gate_name + " q["  + to_string(qubits[0]) + "];\n";
                    break;
                // Parametric 1 qubit gates
                //case cunqa::U1:
                case cunqa::RX:
                case cunqa::RY:
                case cunqa::RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    qasm_circt += gate_name + "(" + std::to_string(params[0]) + ") q[" + to_string(qubits[0]) + "];\n";
                    break;
                //UNITARY
                case cunqa::UNITARY:
                    matrix = instruction.at("params").get<std::vector<std::vector<std::vector<std::vector<double>>>>>();
                    qasm_circt += gate_name + "(" + triple_vector_to_string(matrix[0]) + ") q[" + to_string(qubits[0]) + "];\n";
                    break;
                // 2-Parametric 1 qubit gates
                //case cunqa::U2:
                //case cunqa::R:
                //    qasm_circt += gate_name + "(" + to_string(params[0]) + ", " + to_string(params[1]) + ") q[" + to_string(qubits[0]) + "];\n";
                //    break;
                // 3-Parametric 1 qubit gate
                //case cunqa::U3:
                //    qasm_circt += gate_name + "(" + to_string(params[0]) + ", " + to_string(params[1]) + ", " + to_string(params[2]) + ") q[" + to_string(qubits[0]) + "];\n";
                //    break;
                // Non-parametric 2 qubit gates
                case cunqa::CX:
                case cunqa::CY:
                case cunqa::CZ:
                //case cunqa::CSX:
                //case cunqa::SWAP:
                case cunqa::ECR:
                    qasm_circt += gate_name + " q[" + to_string(qubits[0]) + "], q[" + to_string(qubits[1]) + "];\n";
                    break;
                // Parametric 2 qubit gates
                //case cunqa::RXX:
                //case cunqa::CRX:
                //case cunqa::CRY:
                //case cunqa::CRZ:
                //case cunqa::CP:
                //    qasm_circt += gate_name + "(" + to_string(params[0]) + ")" + " q[" + to_string(qubits[0]) + "], q[" + to_string(qubits[1]) + "];\n";
                //    break;
                // Non-parametric  3 qubit gates
                //case cunqa::CCX:
                //case cunqa::CSWAP:
                //    qasm_circt += gate_name + " q[" + to_string(qubits[0]) + "], q[" + to_string(qubits[1]) + "], q[" + to_string(qubits[2]) + "];\n";
                //    break;
                // Measure, duh
                case cunqa::MEASURE:
                    qasm_circt += "measure q[" + to_string(qubits[0]) + "] -> meas[" + to_string(instruction.at("memory")[0]) + "];\n";
                    break;
                default:
                    std::cout << "Error. Invalid gate name" << "\n";
                    break;
            }

        } catch (const std::exception& e) {
            //SPDLOG_LOGGER_ERROR(logger, "Error translating a gate from JSON to QASM2.");
            std::cout << "Error. Something didn't work." << "\n";
        }
    }
    return qasm_circt;
}
}
