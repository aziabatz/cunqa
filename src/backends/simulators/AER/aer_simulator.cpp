#include "aer_simple_simulator.hpp"
#include "aer_classical_comm_simulator.hpp"

#include <chrono>

#include "simulators/circuit_executor.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"
#include "controllers/state_controller.hpp"
#include "aer_helpers.hpp"

#include "classical_channel.hpp"
#include "utils/json.hpp"
#include "utils/constants.hpp"
#include "logger.hpp"

using namespace std::string_literals;
namespace {
    const auto store = getenv("STORE");
    const std::string filepath = store + "/.cunqa/communications.json"s;
}

using namespace AER;

namespace cunqa {
namespace sim {

AerCCSimulator::AerCCSimulator() : classical_channel(std::make_unique<comm::ClassicalChannel>())
{
    JSON communications_endpoint = 
    {
        {"communications_endpoint", classical_channel->endpoint}
    };
    write_on_file(communications_endpoint, filepath);
};

// Free function used in both simple and distributed case
template <class BackendType>
JSON usual_execution_(const BackendType& backend, const QuantumTask& quantum_task)
{
    try {
        //TODO: Maybe improve them to send several circuits at once
        auto aer_quantum_task = quantum_task_to_AER(quantum_task);
        JSON circuit_json = aer_quantum_task.circuit;

        Circuit circuit(circuit_json);
        std::vector<std::shared_ptr<Circuit>> circuits;
        circuits.push_back(std::make_shared<Circuit>(circuit));

        JSON run_config_json(aer_quantum_task.config);

        // TODO: See how to manage the seeds
        //run_config_json["seed_simulator"] = run_config.seed;
        Config aer_config(run_config_json);

        //LOGGER_DEBUG("circuit: {}.", backend.config.noise_model);
        Noise::NoiseModel noise_model(backend.config.noise_model);

        Result result = controller_execute<Controller>(circuits, noise_model, aer_config);

        return result.to_json();
    } catch (const std::exception& e) {
        // TODO: specify the circuit format in the docs.
        LOGGER_ERROR("Error executing the circuit in the AER simulator.\n\tTry checking the format of the circuit sent and/or of the noise model.");
        return {{"ERROR", std::string(e.what())}};
    }
    return {};
}

template <class BackendType>
JSON dynamic_execution_(const BackendType& backend, const QuantumTask& quantum_task, comm::ClassicalChannel* classical_channel = nullptr)
{
    LOGGER_DEBUG("Starting dynamic_execution_ on Aer.");
    // Add the classical channel
    if (classical_channel) {
        std::vector<std::string> connect_with = quantum_task.sending_to;
        classical_channel->set_classical_connections(connect_with);
        LOGGER_DEBUG("Classical channel ready.");
    }

    std::vector<JSON> instructions = quantum_task.circuit;
    JSON run_config = quantum_task.config;
    uint_t n_qubits = run_config.at("num_qubits").get<uint_t>();
    int shots = run_config.at("shots");
    std::string instruction_name;
    std::vector<uint_t> qubits;
    std::vector<std::uint64_t> clreg;
    std::vector<std::uint64_t> conditional_reg;
    std::vector<std::string> endpoint;
    std::vector<double> params;
    uint_t measurement;
    int measurement_as_int;
    std::map<std::string, std::size_t> measurementCounter;
    JSON result;
    float time_taken;

    std::map<std::size_t, bool> classicValues; // To mimic the way Munich counts
    std::map<std::size_t, bool> classicRegister;
    AER::AerState *state = new AER::AerState();
    state->configure("method", "statevector");
    state->configure("device", "CPU");
    state->configure("precision", "double");
    
    LOGGER_DEBUG("AER variables ready.");

    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {
        
        auto qubit_ids = state->allocate_qubits(n_qubits);
        state->initialize();

        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name").get<std::string>();
            qubits = instruction.at("qubits").get<std::vector<uint_t>>();
            switch (constants::INSTRUCTIONS_MAP.at(instruction_name))
            {
                case constants::MEASURE:
                    clreg = instruction.at("clreg").get<std::vector<std::uint64_t>>();
                    measurement = state->apply_measure(qubits);
                    classicValues[qubits[0]] = (measurement == 1);
                    if (!clreg.empty()) {
                        classicRegister[clreg[0]] = (measurement == 1);
                    }
                    break;
                case constants::ID:
                    break;
                case constants::X:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcx(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcx(qubits);
                        }
                    }
                    break;
                case constants::Y:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcy(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcy(qubits);
                        }
                    }
                    break;
                case constants::Z:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcz(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcz(qubits);
                        }
                    }
                    break;
                case constants::H:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_h(qubits[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_h(qubits[0]);
                        }
                    }
                    break;
                case constants::SX:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcsx(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcsx(qubits);
                        }
                    }
                    break;
                case constants::CX:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcx(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcx(qubits);
                        }
                    }
                    break;
                case constants::CY:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcy(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcy(qubits);
                        }
                    }
                    break;
                case constants::CZ:
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcz(qubits);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcz(qubits);
                        }
                    }
                    break;
                case constants::ECR:
                    // TODO
                    break;
                case constants::RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcrx(qubits, params[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcrx(qubits, params[0]);
                        }
                    }
                    break;
                case constants::RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcry(qubits, params[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcry(qubits, params[0]);
                        }
                    }
                    break;
                case constants::RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcrz(qubits, params[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcrz(qubits, params[0]);
                        }
                    }
                    break;
                case constants::CRX:
                    params = instruction.at("params").get<std::vector<double>>();
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcrx(qubits, params[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcrx(qubits, params[0]);
                        }
                    }
                    break;
                case constants::CRY:
                    params = instruction.at("params").get<std::vector<double>>();
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcry(qubits, params[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcry(qubits, params[0]);
                        }
                    }
                    break;
                case constants::CRZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    if (!instruction.contains("conditional_reg")) {
                        state->apply_mcrz(qubits, params[0]);
                    } else {
                        conditional_reg = instruction.at("conditional_reg").get<std::vector<std::uint64_t>>();
                        if (classicRegister[conditional_reg[0]]) {
                            state->apply_mcrz(qubits, params[0]);
                        }
                    }
                    break;
                case constants::C_IF_H:
                case constants::C_IF_X:
                case constants::C_IF_Y:
                case constants::C_IF_Z:
                case constants::C_IF_CX:
                case constants::C_IF_CY:
                case constants::C_IF_CZ:
                case constants::C_IF_ECR:
                case constants::C_IF_RX:
                case constants::C_IF_RY:
                case constants::C_IF_RZ:
                    // //TODO: Look how Aer natively applies C_IFs operations
                    break;
                case constants::MEASURE_AND_SEND:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = state->apply_measure(qubits);
                    measurement_as_int = static_cast<int>(measurement);
                    classical_channel->send_measure(measurement_as_int, endpoint[0]); 
                    break;
                case constants::REMOTE_C_IF_X:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcx(qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_Y:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcy(qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_Z:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcz(qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_H:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_h(qubits[0]);
                    }
                    break;
                case constants::REMOTE_C_IF_CX:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcx(qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_CY:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcy(qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_CZ:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcz(qubits);
                    }
                    break;
                case constants::REMOTE_C_IF_ECR:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        // TODO
                    }
                    break;
                case constants::REMOTE_C_IF_RX:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    params = instruction.at("params").get<std::vector<double>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcrx(qubits, params[0]);
                    }
                    break;
                case constants::REMOTE_C_IF_RY:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    params = instruction.at("params").get<std::vector<double>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    if (measurement == 1) {
                        state->apply_mcry(qubits, params[0]);
                    }
                    break;
                case constants::REMOTE_C_IF_RZ:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    params = instruction.at("params").get<std::vector<double>>();
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == 1) {
                        state->apply_mcrz(qubits, params[0]);
                    }
                    break;  
                default:
                    LOGGER_ERROR("Invalid gate name."); 
                    throw std::runtime_error("Invalid gate name.");
                    break;
            }
        } // End one shot
        std::string resultString(n_qubits, '0');
        // result is a map from the cbit index to the Boolean value
        for (const auto& [bitIndex, value] : classicValues) {
            resultString[n_qubits - bitIndex - 1] = value ? '1' : '0';
        }
        measurementCounter[resultString]++;

        classicRegister.clear();
        state->clear();
    } // End all shots

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = stop_time - start_time;
    time_taken = duration.count();

    result = {
        {"counts", measurementCounter},
        {"time_taken", time_taken}
    }; 

    return result;
}

// Simple AerSimulator
AerSimpleSimulator::~AerSimpleSimulator() = default;

JSON AerSimpleSimulator::execute(const SimpleBackend& backend, const QuantumTask& quantum_task) 
{
    if (!quantum_task.is_dynamic) {
        return usual_execution_<SimpleBackend>(backend, quantum_task);
    } else {
        return dynamic_execution_<SimpleBackend>(backend, quantum_task);
    } 
}

// Distributed AerSimulator
JSON AerCCSimulator::execute(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    if (!quantum_task.is_dynamic) {
        return usual_execution_<ClassicalCommBackend>(backend, quantum_task);
    } else {
        return dynamic_execution_<ClassicalCommBackend>(backend, quantum_task, this->classical_channel.get());
    } 
}





} // End of sim namespace
} // End of cunqa namespace