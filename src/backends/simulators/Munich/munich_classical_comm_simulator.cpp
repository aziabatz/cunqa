#include "munich_classical_comm_simulator.hpp"

#include <chrono>
#include <optional>

#include "utils/constants.hpp"
#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {

// TODO
std::map<std::size_t, bool> DistributedCircuitSimulator::singleShot(bool ignoreNonUnitaries)
{
    std::map<std::size_t, bool> res;
    return res;
}


// Classical Communications QuantumComputation
void QuantumComputation::classicControlledDistributed(const qc::OpType op, std::string& sending_endpoint, std::string& receiving_endpoint, qc::Qubit& control_qubit, qc::Targets& target_qubits, const std::vector<qc::fp>& params)
{
    this->emplace_back<DistributedClassicalCommunicationOperation>(op, sending_endpoint, receiving_endpoint, control_qubit, target_qubits, params);
}



void QuantumComputation::set_circuit()
{
    std::vector<JSON> instructions = this->circuit;
    std::string instruction_name;
    std::vector<std::uint32_t> qubits;
    std::vector<std::uint32_t> targets;
    std::vector<std::uint64_t> clbits;
    std::vector<std::string> endpoints;
    std::vector<double> params;
    qc::ClassicalRegister clreg; 
    std::size_t size = sizeof(std::size_t);

    for (auto& instruction : instructions) {
        instruction_name = instruction.at("name");
        qubits = instruction.at("qubits").get<std::vector<std::uint32_t>>();

    // TODO: Try to make this switch with MACROS (check QuantumComputation.hpp)
        switch (cunqa::constants::INSTRUCTIONS_MAP.at(instruction_name))
        {
            case cunqa::constants::MEASURE:
                clbits = instruction.at("memory").get<std::vector<std::uint64_t>>();
                this->measure(qubits[0], clbits[0]);
                break;
            case cunqa::constants::X:
                this->x(qubits[0]);
                break;
            case cunqa::constants::Y:
                this->y(qubits[0]);
                break;
            case cunqa::constants::Z:
            this->z(qubits[0]);
                break;
            case cunqa::constants::H:
                this->h(qubits[0]);
                break;
            case cunqa::constants::SX:
                this->sx(qubits[0]);
                break;
            case cunqa::constants::RX:
                params = instruction.at("params").get<std::vector<double>>();
                this->rx(params[0], qubits[0]);
                break;
            case cunqa::constants::RY:
                params = instruction.at("params").get<std::vector<double>>();
                this->ry(params[0], qubits[0]);
                break;
            case cunqa::constants::RZ:
                params = instruction.at("params").get<std::vector<double>>();
                this->rz(params[0], qubits[0]);
                break;
            case cunqa::constants::CX:
                this->cx(qc::Control(qubits[0]), qubits[1]);
                break;
            case cunqa::constants::CY:
                this->cy(qc::Control(qubits[0]), qubits[1]);
                break;
            case cunqa::constants::CZ:
                this->cz(qc::Control(qubits[0]), qubits[1]);
                break;
            case cunqa::constants::CRX:
                params = instruction.at("params").get<std::vector<double>>();
                this->crx(params[0], qc::Control(qubits[0]), qubits[1]);
                break;
            case cunqa::constants::CRY:
                params = instruction.at("params").get<std::vector<double>>();
                this->cry(params[0], qc::Control(qubits[0]), qubits[1]);
                break;
            case cunqa::constants::CRZ:
                params = instruction.at("params").get<std::vector<double>>();
                this->crz(params[0], qc::Control(qubits[0]), qubits[1]);
                break;
            case cunqa::constants::ECR:
                this->ecr(qubits[0], qubits[1]);
                break;
            case cunqa::constants::CECR:
                this->cecr(qc::Control(qubits[0]), qubits[1], qubits[2]);
                break;
            case cunqa::constants::C_IF_H:
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::H, qubits[0], clreg);
                break;
            case cunqa::constants::C_IF_X:
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::X, qubits[0], clreg);
                break;
            case cunqa::constants::C_IF_Y:
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::Y, qubits[0], clreg);
                break;
            case cunqa::constants::C_IF_Z:
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::Z, qubits[0], clreg);
                break;
            case cunqa::constants::C_IF_RX:
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                params = instruction.at("params").get<std::vector<double>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::RX, qubits[0], clreg, 1U, params);
                break;
            case cunqa::constants::C_IF_RY:
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                params = instruction.at("params").get<std::vector<double>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::RY, qubits[0], clreg, 1U, params);
                break;
            case cunqa::constants::C_IF_RZ:
                params = instruction.at("params").get<std::vector<double>>();
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::RZ, qubits[0], clreg, 1U, params);
                break;
            case cunqa::constants::REMOTE_C_IF_H:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                this->classicControlledDistributed(qc::OpType::H, endpoints[0], endpoints[1], qubits[0], targets); // (gate[OpType], send_endpoints[std::string], recv_endpoints[std::string], class_control_qubit[int], target_qubit[std::vector<int>], params[std::vector<double>])
                break;
            case cunqa::constants::REMOTE_C_IF_X:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                this->classicControlledDistributed(qc::OpType::X, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            case cunqa::constants::REMOTE_C_IF_Y:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                this->classicControlledDistributed(qc::OpType::Y, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            case cunqa::constants::REMOTE_C_IF_Z:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                this->classicControlledDistributed(qc::OpType::Z, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            case cunqa::constants::REMOTE_C_IF_RX:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                params = instruction.at("params").get<std::vector<double>>();
                this->classicControlledDistributed(qc::OpType::RX, endpoints[0], endpoints[1], qubits[0], targets, params);
                break;
            case cunqa::constants::REMOTE_C_IF_RY:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                params = instruction.at("params").get<std::vector<double>>();
                this->classicControlledDistributed(qc::OpType::RY, endpoints[0], endpoints[1], qubits[0], targets, params);
                break;
            case cunqa::constants::REMOTE_C_IF_RZ:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1]};
                params = instruction.at("params").get<std::vector<double>>();
                this->classicControlledDistributed(qc::OpType::RZ, endpoints[0], endpoints[1], qubits[0], targets, params);
                break;
            case cunqa::constants::REMOTE_C_IF_CX:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1], qubits[2]};
                this->classicControlledDistributed(qc::OpType::X, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            case cunqa::constants::REMOTE_C_IF_CY:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1], qubits[2]};
                this->classicControlledDistributed(qc::OpType::Y, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            case cunqa::constants::REMOTE_C_IF_CZ:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1], qubits[2]};
                this->classicControlledDistributed(qc::OpType::Z, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            case cunqa::constants::REMOTE_C_IF_ECR:
                this->has_classic_communications = true;
                endpoints = instruction.at("circuits").get<std::vector<std::string>>();
                targets = {qubits[1], qubits[2]};
                this->classicControlledDistributed(qc::OpType::ECR, endpoints[0], endpoints[1], qubits[0], targets);
                break;
            default:
                std::cerr << "Instruction not suported!" << "\n";
        } // End switch
    } // End for 
} // End set_circuit() method



JSON execute(QuantumTask& quantum_task)
{
    /* try {
        LOGGER_DEBUG("Noise JSON: {}", noise_model_json.dump(4));

        std::string circuit(circuit_json.at("instructions"));
        LOGGER_DEBUG("Circuit JSON: {}", circuit);
        auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

        JSON result_json;
        float time_taken;
        LOGGER_DEBUG("Noise JSON: {}", noise_model_json.dump(4));

        if (!noise_model_json.empty()){
            const ApproximationInfo approx_info{noise_model_json["step_fidelity"], noise_model_json["approx_steps"], ApproximationInfo::FidelityDriven};
            StochasticNoiseSimulator sim(std::move(mqt_circuit), approx_info, run_config.seed, "APD", noise_model_json["noise_prob"],
                                            noise_model_json["noise_prob_t1"], noise_model_json["noise_prob_multi"]);
            auto start = std::chrono::high_resolution_clock::now();
            auto result = sim.simulate(run_config.shots);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            time_taken = duration.count();
            !result.empty() ? result_json = JSON(result) : throw std::runtime_error("QASM format is not correct.");
        } else {
            CircuitSimulator sim(std::move(mqt_circuit));
            auto start = std::chrono::high_resolution_clock::now();
            auto result = sim.simulate(run_config.shots);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> duration = end - start;
            time_taken = duration.count();
            !result.empty() ? result_json = JSON(result) : throw std::runtime_error("QASM format is not correct.");
        }        

        LOGGER_DEBUG("Results: {}", result_json.dump(4));
        return JSON({{"counts", result_json}, {"time_taken", time_taken}});
    } catch (const std::exception& e) {
        // TODO: specify the circuit format in the docs.
        LOGGER_ERROR("Error executing the circuit in the Munich simulator.\nTry checking the format of the circuit sent and/or of the noise model.");
        return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
    } */
    return {};
}

// Free execution function
JSON execute(JSON& circuit, int& shots){
    std::map<JSON, int> result;

    std::unique_ptr<QuantumComputation> qc = std::make_unique<QuantumComputation>(circuit);
    CircuitSimulator circsim(std::move(qc));

    if (!qc->has_classic_communications) {
        /* circsim.initializeSimulation(circsim.getNumberOfQubits());
        for (size_t i = 0; i < shots; i++) {
            JSON single_shot_result = circsim.singleShot(false);
            result[single_shot_result]++;
        }
        return result; */

        std::cout << "Nothing to see here" << "\n";

    } else {
        std::cout << "Not yet classical communications" << "\n";
    }
    
    return {};
}

JSON MunichCCSimulator::execute(const SimpleBackend& backend, const QuantumTask& circuit)
{
    return {};
} 


} // End namespace sim
} // End namespace cunqa