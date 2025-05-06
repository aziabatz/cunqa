#include "munichsimulator.hpp"

#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include "comm/qpu_comm.hpp"
#include "config/backend_config.hpp"
#include "utils/constants.hpp"
#include "utils/json.hpp"
#include "logger/logger.hpp"

#include "simulator.hpp"


namespace cunqa 
{

std::map<std::size_t, bool> singleShot(bool ignoreNonUnitaries) override
{
    
}


// Classical Communications QuantumComputation
void QuantumComputation::classicControlledDistributed(const qc::OpType op, std::string& sending_endpoint, std::string& receiving_endpoint, qc::Qubit& control_qubit, qc::Targets& target_qubits, const std::vector<qc::fp>& params)
{
    this->emplace_back<DistributedClassicalCommunicationOperation>(op, sending_endpoint, receiving_endpoint, control_qubit, target_qubits, params);
}



void QuantumComputation::set_circuit()
{
    std::vector<JSON> instructions = this->circuit.at("instructions").get<std::vector<JSON>>();
    std::string instruction_name;
    std::vector<int> qubits;
    std::vector<int> clbits;
    std::vector<std::string> endpoints;
    std::vector<double> params;
    qc::ClassicalRegister clreg; 
    int size = sizeof(std::size_t);

    for (auto& instruction : instructions) {
        instruction_name = op.at("name");
        qubits = op.at("qubits").get<std::vector<int>>();

    // TODO: Try to make this switch with MACROS (check QuantumComputation.hpp)
        switch (CUNQA::INSTRUCTIONS_MAP[instruction_name])
        {
            case CUNQA::MEASURE:
                clbits = op.at("memory").get<std::vector<int>>();
                this->measure(qubits[0], clbits[0]);
                break;
            case CUNQA::X:
                this->x(qubits[0]);
                break;
            case CUNQA::Y:
                this->y(qubits[0]);
                break;
            case CUNQA::Z:
            this->z(qubits[0]);
                break;
            case CUNQA::H:
                this->h(qubits[0]);
                break;
            case CUNQA::SX:
                this->sx(qubits[0]);
                break;
            case CUNQA::RX:
                params = op.at("params").get<std::vector<double>>();
                this->rx(params[0], qubits[0]);
                break;
            case CUNQA::RY:
                params = op.at("params").get<std::vector<double>>();
                this->ry(params[0], qubits[0]);
                break;
            case CUNQA::RZ:
                params = op.at("params").get<std::vector<double>>();
                this->rz(params[0], qubits[0]);
                break;
            case CUNQA::CX:
                this->cx(qc::Control(qubits[0]), qubits[1]);
                break;
            case CUNQA::CY:
                this->cy(qc::Control(qubits[0]), qubits[1]);
                break;
            case CUNQA::CZ:
                this->cz(qc::Control(qubits[0]), qubits[1]);
                break;
            case CUNQA::CRX:
                params = op.at("params").get<std::vector<double>>();
                this->crx(params[0], qc::Control(qubits[0]), qubits[1]);
                break;
            case CUNQA::CRY:
                params = op.at("params").get<std::vector<double>>();
                this->cry(params[0], qc::Control(qubits[0]), qubits[1]);
                break;
            case CUNQA::CRZ:
                params = op.at("params").get<std::vector<double>>();
                this->crz(params[0], qc::Control(qubits[0]), qubits[1]);
                break;
            case CUNQA::ECR:
                this->ecr(qubits[0], qubits[1]);
                break;
            case CUNQA::CECR:
                this->cecr(qc::Control(qubits[0]), qubits[1], qubits[2]);
                break;
            case CUNQA::C_IF_H:
                clbits = op.at("clbits").get<std::vector<int>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::H, qubits[0], clreg);
                break;
            case CUNQA::C_IF_X:
                clbits = op.at("clbits").get<std::vector<int>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::X, qubits[0], clreg);
                break;
            case CUNQA::C_IF_Y:
                clbits = op.at("clbits").get<std::vector<int>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::Y, qubits[0], clreg);
                break;
            case CUNQA::C_IF_Z:
                clbits = op.at("clbits").get<std::vector<int>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::Z, qubits[0], clreg);
                break;
            case CUNQA::C_IF_RX:
                clbits = op.at("clbits").get<std::vector<int>>();
                params = op.at("params").get<std::vector<double>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::RX, qubits[0], clreg, 1U, params);
                break;
            case CUNQA::C_IF_RY:
                clbits = op.at("clbits").get<std::vector<int>>();
                params = op.at("params").get<std::vector<double>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::RY, qubits[0], clreg, 1U, params);
                break;
            case CUNQA::C_IF_RZ:
                params = op.at("params").get<std::vector<double>>();
                clbits = op.at("clbits").get<std::vector<int>>();
                clreg = std::make_pair(clbits[0], size);
                this->classicControlled(qc::OpType::RZ, qubits[0], clreg, 1U, params);
                break;
            case CUNQA::D_C_IF_H:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::H, endpoints[0], endpoints[1], qubits[0], {qubits[1]}); // (gate[OpType], send_endpoints[std::string], recv_endpoints[std::string], class_control_qubit[int], target_qubit[std::vector<int>], params[std::vector<double>])
                break;
            case CUNQA::D_C_IF_X:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::X, endpoints[0], endpoints[1], qubits[0], {qubits[1]});
                break;
            case CUNQA::D_C_IF_Y:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::Y, endpoints[0], endpoints[1], qubits[0], {qubits[1]});
                break;
            case CUNQA::D_C_IF_Z:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::Z, endpoints[0], endpoints[1], qubits[0], {qubits[1]});
                break;
            case CUNQA::D_C_IF_RX:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                params = op.at("params").get<std::vector<double>>();
                this->classicControlledDistributed(qc::OpType::RX, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, params);
                break;
            case CUNQA::D_C_IF_RY:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                params = op.at("params").get<std::vector<double>>();
                this->classicControlledDistributed(qc::OpType::RY, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, params);
                break;
            case CUNQA::D_C_IF_RZ:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                params = op.at("params").get<std::vector<double>>();
                this->classicControlledDistributed(qc::OpType::RZ, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, params);
                break;
            case CUNQA::D_C_IF_CX:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::X, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, {qubits[2]});
                break;
            case CUNQA::D_C_IF_CY:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::Y, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, {qubits[2]});
                break;
            case CUNQA::D_C_IF_CZ:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::Z, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, {qubits[2]});
                break;
            case CUNQA::D_C_IF_ECR:
                this->has_classic_communications = true;
                endpoints = op.at("circuits").get<std::vector<std::string>>();
                this->classicControlledDistributed(qc::OpType::ECR, endpoints[0], endpoints[1], qubits[0], {qubits[1], qubits[2]});
                break;
            default:
                std::cerr << "Instruction not suported!" << "\n";
        } // End switch
    } // End for 
} // End set_circuit() method



JSON execute(JSON& circuit_json, JSON& noise_model_json,  const config::RunConfig& run_config)
{
    try {
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
    }
    return {};
}

// Free execution function
JSON execute(JSON& circuit, int& shots){
    std::map<JSON, int> result;

    std::unique_ptr<QuantumComputation> qc = std::make_unique<QuantumComputation>(circuit);
    qc::CircuitSimulator circsim(std::move(qc));

    if (!qc->has_classic_communications) {
        circsim.initializeSimulation(circsim.getNumberOfQubits());
        for (size_t i = 0; i < shots; i++) {
            JSON single_shot_result = circsim.singleShot(false);
            result[single_shot_result]++;
        }
        return result;

    } else {
        std::cout << "Not yet classical communications" << "\n";
    }
    
}


} // End namespace cunqa