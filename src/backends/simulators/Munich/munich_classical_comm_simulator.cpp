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
    JSON result;
    if (!circuit.is_distributed) {
        result = usual_execution(backend, circuit);
        return result;
    } else {
        result = distributed_execution(backend, circuit);
        return result;
    }   
} 


std::string MunichCCSimulator::_get_communication_endpoint()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}


// Free functions
JSON usual_execution(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
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

JSON distributed_execution(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("Munich distributed execution");
    JSON result_json;
    float time_taken;
    std::vector<JSON> instructions = quantum_task.circuit;
    JSON run_config = quantum_task.config;
    int shots = quantum_task.config.at("shots").get<int>();
    std::string instruction_name;
    std::vector<int> clbits;
    std::vector<uint16_t> qubits;
    qc::ClassicalRegister clreg; 
    std::vector<std::string> endpoint;
    std::vector<double> params;
    char measurement;
    std::unique_ptr<qc::Operation> std_op;
    std::unique_ptr<qc::Operation> c_op;
    std::unique_ptr<qc::Control> pControl;

    std::unique_ptr<ClassicalCommQuantumComputation> qc = std::make_unique<ClassicalCommQuantumComputation>(quantum_task);
    ClassicalCommCircuitSimulator CCcircsim(std::move(qc));

    const std::size_t nQubits = quantum_task.config.at("num_qubits").get<std::size_t>();

    CCcircsim.CCinitializeSimulation(nQubits);

    std::map<std::size_t, bool> classicValues;

    for (int i = 0; i < shots; i++) {
        for (auto& instruction : instructions) {
            instruction_name = instruction.at("name");
            qubits = instruction.at("qubits").get<std::vector<std::uint16_t>>();

            switch (cunqa::constants::INSTRUCTIONS_MAP.at(instruction_name))
            {
                case cunqa::constants::MEASURE:
                    measurement = CCcircsim.CCmeasure(qubits[0]);
                    classicValues[qubits[0]] = (measurement == '1');
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
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::H);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_X:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::X);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_Y:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::Y);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_Z:
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::Z);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RX, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RY, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::C_IF_RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::RZ, params);
                    c_op = std::make_unique<qc::ClassicControlledOperation>(std_op, qubits[0]);
                    CCcircsim.CCapplyOperationToState(c_op);
                    break;
                case cunqa::constants::MEASURE_AND_SEND:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    measurement = CCcircsim.CCmeasure(qubits[0]);
                    LOGGER_DEBUG("Trying to send to {}", endpoint[0]);
                    classical_channel->send_measure(measurement, endpoint[0]); 
                    LOGGER_DEBUG("Measurement sent to {}", endpoint[0]);
                    break;
                case cunqa::constants::REMOTE_C_IF_H:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::H);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_X:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::X);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Y:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Y);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_Z:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::Z);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RX:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RX, params);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RY:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RY, params);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_RZ:
                    params = instruction.at("params").get<std::vector<double>>();
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[0], qc::OpType::RZ, params);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CX:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::X);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CY:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Y);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_CZ:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        pControl = std::make_unique<qc::Control>(qubits[0]);
                        std_op = std::make_unique<qc::StandardOperation>(*pControl, qubits[1], qc::OpType::Z);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                case cunqa::constants::REMOTE_C_IF_ECR:
                    endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                    LOGGER_DEBUG("Origin endpoint: {}", endpoint[0]);
                    measurement = classical_channel->recv_measure(endpoint[0]); 
                    LOGGER_DEBUG("Measurement received from {}", endpoint[0]);
                    if (measurement == '1') {
                        std_op = std::make_unique<qc::StandardOperation>(qubits[1], qc::OpType::ECR);
                        CCcircsim.CCapplyOperationToState(std_op);
                    }
                    break;
                default:
                    std::cerr << "Instruction not suported!" << "\n";
            } // End switch
        } // End for 
    }
    
    result_json = classicValues;
    return result_json;
}

/* JSON distributed_execution(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the distributed_execution() method.");
    std::map<std::string, std::size_t> measurementCounter;

    std::unique_ptr<ClassicalCommQuantumComputation> qc = std::make_unique<ClassicalCommQuantumComputation>(quantum_task);
    ClassicalCommCircuitSimulator CCcircsim(std::move(qc));

    int shots = quantum_task.config.at("shots").get<int>();

    for (int i = 0; i < shots; i++) {
        const std::map<std::size_t, bool> result = CCcircsim.CCsingleShot();
        const std::size_t cbits = qc->getNcbits();

        std::string resultString(qc->getNcbits(), '0');

        // result is a map from the cbit index to the Boolean value
        for (const auto& [bitIndex, value] : result) {
        resultString[cbits - bitIndex - 1] = value ? '1' : '0';
        }
        measurementCounter[resultString]++;
    }

    return measurementCounter;
} */


// TODO
std::map<std::size_t, bool> ClassicalCommCircuitSimulator::CCsingleShot()
{
    singleShots++;
    const std::size_t nQubits = qc->getNqubits();

    initializeSimulation(nQubits);

    std::size_t opNum = 0;
    std::map<std::size_t, bool> classicValues;

    const auto approxMod = static_cast<std::size_t>(
        std::ceil(static_cast<double>(qc->getNops()) /
                (static_cast<double>(approximationInfo.stepNumber + 1))));

    for (auto& op : *qc) {
    if (op->isNonUnitaryOperation()) {
        if (auto* nonUnitaryOp =
                dynamic_cast<qc::NonUnitaryOperation*>(op.get())) {
        if (op->getType() == qc::Measure) {
            const auto& quantum = nonUnitaryOp->getTargets();
            const auto& classic = nonUnitaryOp->getClassics();

            assert(
                quantum.size() ==
                classic.size()); // this should not happen do to check in Simulate

            for (std::size_t i = 0; i < quantum.size(); ++i) {
                auto result = measure(static_cast<dd::Qubit>(quantum.at(i)));
                assert(result == '0' || result == '1');
                classicValues[classic.at(i)] = (result == '1');
            }

        } else if (nonUnitaryOp->getType() == qc::Reset) {
            reset(nonUnitaryOp);
        /* } else if (op->getType() == cunqa::sim::MeasureAndSend) {
            continue; //TODO */
        } else {
            throw std::runtime_error("Unsupported non-unitary functionality.");
        }
        } else {
        throw std::runtime_error("Dynamic cast to NonUnitaryOperation failed.");
        }
        dd->garbageCollect();
    } else {
        if (op->isClassicControlledOperation()) {
            if (auto* classicallyControlledOp =
                    dynamic_cast<qc::ClassicControlledOperation*>(op.get())) {
                const auto startIndex = static_cast<std::uint16_t>(
                    classicallyControlledOp->getParameter().at(0));
                const auto length = static_cast<std::uint16_t>(
                    classicallyControlledOp->getParameter().at(1));
                const auto expectedValue =
                    classicallyControlledOp->getExpectedValue();
                unsigned int actualValue = 0;
                for (std::size_t i = 0; i < length; i++) {
                actualValue |= (classicValues[startIndex + i] ? 1U : 0U) << i;
                }
                if (actualValue != expectedValue) {
                continue;
                }
            /* } else if (op->getType() == cunqa::sim::RemoteCIf) {
                continue; //TODO */
            } else {
                throw std::runtime_error(
                    "Dynamic cast to ClassicControlledOperation failed.");
            }
        }
        CCapplyOperationToState(op);

        if (approximationInfo.stepNumber > 0 &&
            approximationInfo.stepFidelity < 1.0) {
        if (approximationInfo.strategy == ApproximationInfo::FidelityDriven &&
            (opNum + 1) % approxMod == 0 &&
            approximationRuns < approximationInfo.stepNumber) {
            [[maybe_unused]] const auto sizeBefore = rootEdge.size();
            const auto apFid = approximateByFidelity(
                approximationInfo.stepFidelity, false, true);
            approximationRuns++;
            finalFidelity *= static_cast<long double>(apFid);
        } else if (approximationInfo.strategy ==
                    ApproximationInfo::MemoryDriven) {
            [[maybe_unused]] const auto sizeBefore = rootEdge.size();
            if (dd->template getUniqueTable<dd::vNode>()
                    .possiblyNeedsCollection()) {
            const auto apFid = approximateByFidelity(
                approximationInfo.stepFidelity, false, true);
            approximationRuns++;
            finalFidelity *= static_cast<long double>(apFid);
            }
        }
        }
        dd->garbageCollect();
    }
    opNum++;
    }
    return classicValues;
}


// Classical Communications QuantumComputation
void ClassicalCommQuantumComputation::measure_and_send(std::string& sending_endpoint, qc::Qubit& control_qubit)
{
    //this->emplace_back<MeasureAndSend>(sending_endpoint, control_qubit);
}

void ClassicalCommQuantumComputation::remote_c_if(const qc::OpType op, std::string& receiving_endpoint, qc::Targets& target_qubits, const std::vector<qc::fp>& params)
{
    //this->emplace_back<RemoteCIf>(op, receiving_endpoint, target_qubits, params);
}


void ClassicalCommQuantumComputation::set_circuit()
{    
    std::vector<JSON> instructions = this->circuit;
    std::string instruction_name;
    std::vector<std::uint32_t> qubits;
    std::vector<std::uint64_t> clbits;
    std::vector<std::string> endpoint;
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
                clbits = instruction.at("clbits").get<std::vector<std::uint64_t>>();
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
            case cunqa::constants::MEASURE_AND_SEND:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->measure_and_send(endpoint[0], qubits[0]);
                break;
            case cunqa::constants::REMOTE_C_IF_H:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::H, endpoint[0], qubits); 
                break;
            case cunqa::constants::REMOTE_C_IF_X:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::X, endpoint[0], qubits);
                break;
            case cunqa::constants::REMOTE_C_IF_Y:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::Y, endpoint[0], qubits);
                break;
            case cunqa::constants::REMOTE_C_IF_Z:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::Z, endpoint[0], qubits);
                break;
            case cunqa::constants::REMOTE_C_IF_RX:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                params = instruction.at("params").get<std::vector<double>>();
                this->remote_c_if(qc::OpType::RX, endpoint[0], qubits, params);
                break;
            case cunqa::constants::REMOTE_C_IF_RY:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                params = instruction.at("params").get<std::vector<double>>();
                this->remote_c_if(qc::OpType::RY, endpoint[0], qubits, params);
                break;
            case cunqa::constants::REMOTE_C_IF_RZ:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                params = instruction.at("params").get<std::vector<double>>();
                this->remote_c_if(qc::OpType::RZ, endpoint[0], qubits, params);
                break;
            case cunqa::constants::REMOTE_C_IF_CX:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::X, endpoint[0], qubits);
                break;
            case cunqa::constants::REMOTE_C_IF_CY:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::Y, endpoint[0], qubits);
                break;
            case cunqa::constants::REMOTE_C_IF_CZ:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::Z, endpoint[0], qubits);
                break;
            case cunqa::constants::REMOTE_C_IF_ECR:
                this->has_classic_communications = true;
                endpoint = instruction.at("qpus").get<std::vector<std::string>>();
                this->remote_c_if(qc::OpType::ECR, endpoint[0], qubits);
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



} // End namespace sim
} // End namespace cunqa

