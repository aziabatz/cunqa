#pragma once


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

class MunichSimulator {
public:
    MunichSimulator()
    {}
    
    void configure_simulator(cunqa::JSON& backend_config)
    {
        LOGGER_DEBUG("No configuration needed for MunichSimulator");
    }

    //Offloading execution
    cunqa::JSON execute(cunqa::JSON circuit_json, cunqa::JSON& noise_model_json,  const config::RunConfig& run_config) 
    {
        try {
            LOGGER_DEBUG("Noise cunqa::JSON: {}", noise_model_json.dump(4));

            std::string circuit(circuit_json.at("instructions"));
            LOGGER_DEBUG("Circuit cunqa::JSON: {}", circuit);
            auto mqt_circuit = std::make_unique<qc::QuantumComputation>(std::move(qc::QuantumComputation::fromQASM(circuit)));

            cunqa::JSON result_json;
            float time_taken;
            LOGGER_DEBUG("Noise cunqa::JSON: {}", noise_model_json.dump(4));

            if (!noise_model_json.empty()){
                const ApproximationInfo approx_info{noise_model_json["step_fidelity"], noise_model_json["approx_steps"], ApproximationInfo::FidelityDriven};
                StochasticNoiseSimulator sim(std::move(mqt_circuit), approx_info, run_config.seed, "APD", noise_model_json["noise_prob"],
                                                noise_model_json["noise_prob_t1"], noise_model_json["noise_prob_multi"]);
                auto start = std::chrono::high_resolution_clock::now();
                auto result = sim.simulate(run_config.shots);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> duration = end - start;
                time_taken = duration.count();
                !result.empty() ? result_json = cunqa::JSON(result) : throw std::runtime_error("QASM format is not correct.");
            } else {
                CircuitSimulator sim(std::move(mqt_circuit));
                auto start = std::chrono::high_resolution_clock::now();
                auto result = sim.simulate(run_config.shots);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> duration = end - start;
                time_taken = duration.count();
                !result.empty() ? result_json = cunqa::JSON(result) : throw std::runtime_error("QASM format is not correct.");
            }        

            LOGGER_DEBUG("Results: {}", result_json.dump(4));
            return cunqa::JSON({{"counts", result_json}, {"time_taken", time_taken}});
        } catch (const std::exception& e) {
            // TODO: specify the circuit format in the docs.
            LOGGER_ERROR("Error executing the circuit in the Munich simulator.\nTry checking the format of the circuit sent and/or of the noise model.");
            return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
        }
        return {};
    }

    
    //Dynamic execution
    inline int _apply_measure(std::array<int, 3>& qubits)
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
        return -1;
    }
    
    inline void _apply_gate(std::string& gate_name, std::array<int, 3>& qubits, std::vector<double>& param)
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
    }

    inline int _get_statevector_nonzero_position()
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
        return -1;
    }

    inline void _reinitialize_statevector()
    {
        LOGGER_ERROR("Error. Dynamic execution is not available with Munich simulator. ");
    }






    void ConfigureQuantumComputation(std::unique_ptr<qc::QuantumComputation> qc, std::vector<cunqa::JSON>& circuit)
    {
        std::string instruction_name;
        std::vector<int> qubits;
        std::vector<std::string> endpoints;
        std::vector<double> params;
        std::vector<int> clbits;
        qc::ClassicalRegister clreg; 
        int size = sizeof(std::size_t); 

        for (auto& op : circuit) {
            instruction_name = op.at("name");
            qubits = op.at("qubits").get<std::vector<int>>();

            // TODO: Try to make this switch with MACROS (check QuantumComputation.hpp)
            switch (CUNQA::INSTRUCTIONS_MAP[instruction_name])
            {
                case CUNQA::MEASURE:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    qc->measure(qubits[0], clbits[0]);
                    break;
                case CUNQA::X:
                    qc->x(qubits[0]);
                    break;
                case CUNQA::Y:
                    qc->y(qubits[0]);
                    break;
                case CUNQA::Z:
                    qc->z(qubits[0]);
                    break;
                case CUNQA::H:
                    qc->h(qubits[0]);
                    break;
                case CUNQA::SX:
                    qc->sx(qubits[0]);
                    break;
                case CUNQA::RX:
                    params = op.at("params").get<std::vector<double>>();
                    qc->rx(params[0], qubits[0]);
                    break;
                case CUNQA::RY:
                    params = op.at("params").get<std::vector<double>>();
                    qc->ry(params[0], qubits[0]);
                    break;
                case CUNQA::RZ:
                    params = op.at("params").get<std::vector<double>>();
                    qc->rz(params[0], qubits[0]);
                    break;
                case CUNQA::CX:
                    qc->cx(qc::Control(qubits[0]), qubits[1]);
                    break;
                case CUNQA::CY:
                    qc->cy(qc::Control(qubits[0]), qubits[1]);
                    break;
                case CUNQA::CZ:
                    qc->cz(qc::Control(qubits[0]), qubits[1]);
                    break;
                case CUNQA::CRX:
                    params = op.at("params").get<std::vector<double>>();
                    qc->crx(params[0], qc::Control(qubits[0]), qubits[1]);
                    break;
                case CUNQA::CRY:
                    params = op.at("params").get<std::vector<double>>();
                    qc->cry(params[0], qc::Control(qubits[0]), qubits[1]);
                    break;
                case CUNQA::CRZ:
                    params = op.at("params").get<std::vector<double>>();
                    qc->crz(params[0], qc::Control(qubits[0]), qubits[1]);
                    break;
                case CUNQA::ECR:
                    qc->ecr(qubits[0], qubits[1]);
                    break;
                case CUNQA::CECR:
                    qc->cecr(qc::Control(qubits[0]), qubits[1], qubits[2]);
                    break;
                case CUNQA::C_IF_H:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::H, qubits[0], clreg);
                    break;
                case CUNQA::C_IF_X:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::X, qubits[0], clreg);
                    break;
                case CUNQA::C_IF_Y:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::Y, qubits[0], clreg);
                    break;
                case CUNQA::C_IF_Z:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::Z, qubits[0], clreg);
                    break;
                case CUNQA::C_IF_RX:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    params = op.at("params").get<std::vector<double>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::RX, qubits[0], clreg, 1U, params);
                    break;
                case CUNQA::C_IF_RY:
                    clbits = op.at("clbits").get<std::vector<int>>();
                    params = op.at("params").get<std::vector<double>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::RY, qubits[0], clreg, 1U, params);
                    break;
                case CUNQA::C_IF_RZ:
                    params = op.at("params").get<std::vector<double>>();
                    clbits = op.at("clbits").get<std::vector<int>>();
                    clreg = std::make_pair(clbits[0], size);
                    qc->classicControlled(qc::OpType::RZ, qubits[0], clreg, 1U, params);
                    break;
                case CUNQA::D_C_IF_H:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::H, endpoints[0], endpoints[1], qubits[0], {qubits[1]}); // (gate[OpType], send_endpoints[std::string], recv_endpoints[std::string], class_control_qubit[int], target_qubit[std::vector<int>], params[std::vector<double>])
                    break;
                case CUNQA::D_C_IF_X:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::X, endpoints[0], endpoints[1], qubits[0], {qubits[1]});
                    break;
                case CUNQA::D_C_IF_Y:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::Y, endpoints[0], endpoints[1], qubits[0], {qubits[1]});
                    break;
                case CUNQA::D_C_IF_Z:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::Z, endpoints[0], endpoints[1], qubits[0], {qubits[1]});
                    break;
                case CUNQA::D_C_IF_RX:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    params = op.at("params").get<std::vector<double>>();
                    qc->classicControlledDistributed(qc::OpType::RX, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, params);
                    break;
                case CUNQA::D_C_IF_RY:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    params = op.at("params").get<std::vector<double>>();
                    qc->classicControlledDistributed(qc::OpType::RY, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, params);
                    break;
                case CUNQA::D_C_IF_RZ:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    params = op.at("params").get<std::vector<double>>();
                    qc->classicControlledDistributed(qc::OpType::RZ, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, params);
                    break;
                case CUNQA::D_C_IF_CX:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::X, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, {qubits[2]});
                    break;
                case CUNQA::D_C_IF_CY:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::Y, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, {qubits[2]});
                    break;
                case CUNQA::D_C_IF_CZ:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::Z, endpoints[0], endpoints[1], qubits[0], {qubits[1]}, {qubits[2]});
                    break;
                case CUNQA::D_C_IF_ECR:
                    endpoints = op.at("circuits").get<std::vector<std::string>>();
                    qc->classicControlledDistributed(qc::OpType::ECR, endpoints[0], endpoints[1], qubits[0], {qubits[1], qubits[2]});
                    break;
                default:
                    std::cerr << "Instruction not suported!" << "\n";
            } // End switch
        } // End for
    } // End AddInstructions

    // Classical Controlled Distribued Execution:
    // Estoy suponiendo que las "instructions" son una lista de instrucciones y no un qasm
    cunqa::JSON ccdexecute(cunqa::JSON circuit_json, const config::RunConfig& run_config)
    {
        // General configuration
        int shots = run_config.shots;
        std::vector<cunqa::JSON> circuit = circuit_json.at("instructions");
        std::size_t n_qubits(circuit_json.at("num_qubits"));
        std::size_t n_cregs(circuit_json.at("config").at("memory"));

        // QuantumComputation object from MQT-Core
        std::unique_ptr<qc::QuantumComputation> qc = std::make_unique<qc::QuantumComputation>(n_qubits, n_cregs);

        

        //Specific configuration of each gate
        std::string instruction_name;

        ConfigureQuantumComputation(qc, circuit);
        



        return {};


    }


};