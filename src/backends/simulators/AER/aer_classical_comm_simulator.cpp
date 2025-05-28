#include "aer_classical_comm_simulator.hpp"

#include "controllers/aer_controller.hpp"
#include "controllers/controller_execute.hpp"
#include "controllers/state_controller.hpp"
#include "framework/circuit.hpp"

#include "aer_helpers.hpp"

#include "utils/json.hpp"
#include "utils/constants.hpp"
#include "logger.hpp"

using namespace AER;

namespace cunqa {
namespace sim {

JSON AerCCSimulator::execute(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the execute() method.");
    if (!quantum_task.is_distributed) {
        return this->usual_execution_(backend, quantum_task);
    } else {
        return this->distributed_execution_(backend, quantum_task);
    } 

}

std::string AerCCSimulator::get_communication_endpoint_()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}

JSON AerCCSimulator::usual_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    try {
        //TODO: Maybe improve them to send several circuits at once
        auto aer_quantum_task = quantum_task_to_AER(quantum_task);
        JSON circuit_json = aer_quantum_task.circuit;

        LOGGER_DEBUG("Circuit {}.", circuit_json.dump(4));

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

JSON AerCCSimulator::distributed_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
     std::vector<std::string> connect_with = quantum_task.sending_to;
    this->classical_channel->set_classical_connections(connect_with);

    std::vector<JSON> instructions = quantum_task.circuit;
    JSON run_config = quantum_task.config;
    uint_t n_qubits = run_config.at("num_qubits").get<uint_t>();
    int shots = run_config.at("shots");
    std::string instruction_name;
    std::vector<uint_t> qubits;
    std::vector<std::string> endpoint;
    std::vector<double> params;
    uint_t measurement;
    int measurement_as_int;
    std::map<std::string, std::size_t> measurementCounter;
    JSON result;
    float time_taken;

    std::map<std::size_t, bool> classicValues; // To mimic the way Munich counts
    AER::AerState *state = new AER::AerState();
    state->configure("method", "statevector");
    state->configure("device", "CPU");
    state->configure("precision", "double");
    auto qubit_ids = state->allocate_qubits(n_qubits);

    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shots; i++) {
        
        state->initialize();

        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name").get<std::string>();
            qubits = instruction.at("qubits").get<std::vector<uint_t>>();
            switch (constants::INSTRUCTIONS_MAP.at(instruction_name))
            {
                case constants::MEASURE:
                    measurement = state->apply_measure(qubits);
                    classicValues[qubits[0]] = (measurement == 1);
                    break;
                case constants::ID:
                    break;
                case constants::X:
                    state->apply_mcx(qubits);
                    break;
                case constants::Y:
                    state->apply_mcy(qubits);
                    break;
                case constants::Z:
                    state->apply_mcz(qubits);
                    break;
                case constants::H:
                    state->apply_h(qubits[0]);
                    break;
                case constants::SX:
                    state->apply_mcsx(qubits);
                    break;
                case constants::CX:
                    state->apply_mcx(qubits);;
                    break;
                case constants::CY:
                    state->apply_mcy(qubits);
                    break;
                case constants::CZ:
                    state->apply_mcz(qubits);
                    break;
                case constants::ECR:
                    // TODO
                    break;
                case constants::C_IF_H:
                    // TODO
                    break;
                case constants::C_IF_X:
                    // TODO
                    break;
                case constants::C_IF_Y:
                    // TODO
                    break;
                case constants::C_IF_Z:
                    // TODO
                    break;
                case constants::C_IF_CX:
                    // TODO
                    break;
                case constants::C_IF_CY:
                    // TODO
                    break;
                case constants::C_IF_CZ:
                    // TODO
                    break;
                case constants::C_IF_ECR:
                    // TODO
                    break;
                case constants::RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    state->apply_mcrx(qubits, params[0]);
                    break;
                case constants::RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    state->apply_mcry(qubits, params[0]);
                    break;
                case constants::RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    state->apply_mcrz(qubits, params[0]);
                    break;
                case constants::CRX:
                    params = instruction.at("params").get<std::vector<double>>();
                    state->apply_mcrx(qubits, params[0]);
                    break;
                case constants::CRY:
                    params = instruction.at("params").get<std::vector<double>>();
                    state->apply_mcry(qubits, params[0]);
                    break;
                case constants::CRZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    state->apply_mcrz(qubits, params[0]);
                    break;
                case constants::C_IF_RX:
                    // TODO
                    break;
                case constants::C_IF_RY:
                    // TODO
                    break;
                case constants::C_IF_RZ:
                    // TODO
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


} // End namespace sim
} // End namespace cunqa