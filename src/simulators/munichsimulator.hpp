#pragma once

#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"
#include "operations/Operation.hpp"
#include "Definitions.hpp"

#include "comm/qpu_comm.hpp"
#include "config/backend_config.hpp"
#include "utils/constants.hpp"
#include "utils/json.hpp"
#include "logger/logger.hpp"

#include "simulator.hpp"

namespace cunqa 
{

class QuantumComputation;

class DistributedCircuitSimulator : public qc::CircuitSimulator
{
public:
    // Constructors
    DistributedCircuitSimulator() = default;
    DistributedCircuitSimulator(std::unique_ptr<QuantumComputation>&& qc_) : qc(std::move(qc_))
    {}

    // Methods
    std::map<std::size_t, bool> singleShot(bool ignoreNonUnitaries) override;

protected:


}


// New operation for Distributed Classical Communications
class DistributedClassicalCommunicationOperation : public qc::Operation
{
public:
    DistributedClassicalCommunicationOperation() = default;
    DistributedClassicalCommunicationOperation(const qc::OpType op, std::string& sending_endpoint, std::string& receiving_endpoint, qc::Qubit& control_qubit, qc::Targets& target_qubits, const std::vector<qc::fp>& params = {})
    {
        type = op;
        sending_endpoint = sending_endpoint;
        receiving_endpoint = receiving_endpoint;
        control_qubit = control_qubit;
        targets.emplace_back(target_qubits);
        parameter = std::move(params);
    }

    [[nodiscard]] std::unique_ptr<Operation> clone() const override {
        return std::make_unique<DistributedClassicalCommunicationOperation>(*this);
    }

    // TODO
    void addControl(const Control c) override {
        std::cout << "Dummy method to override the pure virtual addControl()" << "\n";    
    }

    void clearControls() override { 
        std::cout << "Dummy method to override the pure virtual clearControls()" << "\n"; 
    }

    void removeControl(const Control c) override {
        std::cout << "Dummy method to override the pure virtual removeControl()" << "\n"; 
    }

    Controls::iterator removeControl(const Controls::iterator it) override {
        return controls.erase(it); 
    }

    void dumpOpenQASM(std::ostream& of, const RegisterNames& qreg, const RegisterNames& creg, std::size_t indent, bool openQASM3) const override {
        std::cout << "Dummy method to override the pure virtual dumpOpenQASM()" << "\n";
    }

    void invert() override {
        std::cout << "Dummy method to override the pure virtual invert()" << "\n";
    }

protected:
    std::string sending_endpoint;
    std::string receiving_endpoint;
    qc::Qubit control_qubit;

}


// Extension of qc::QuantumComputation for Distributed Classical Communications
class QuantumComputation : public qc::QuantumComputation
{
public:
    // Constructors
    QuantumComputation() = default;
    QuantumComputation(std::string& qasm_qc) : qc::QuantumComputation(qc::QuantumComputation::fromQASM(qasm_qc))
    {
        LOGGER_DEBUG("qasm circuit constructor");
    }
    QuantumComputation(JSON& circuit) : qc::QuantumComputation(circuit.at("num_qubits").get<int>(), circuit.at("num_clbits").get<int>()), circuit(circuit)
    {
        LOGGER_DEBUG("json circuit constructor");
        this->set_circuit();
    }

    // Methods
    void classicControlledDistributed(const qc::OpType op, std::string& sending_endpoint, std::string& receiving_endpoint, qc::Qubit& control_qubit, qc::Targets& target_qubits, const std::vector<qc::fp>& params = {});

    void set_circuit();


protected:
    bool has_classic_communications = false;
    JSON circuit;

}

// Execution without communications and with qasm
JSON execute(JSON& circuit_json, JSON& noise_model_json,  const config::RunConfig& run_config);

// Dynamic execution (for classical communications)
JSON execute(JSON& circuit, int& shots);




} // End namespace cunqa