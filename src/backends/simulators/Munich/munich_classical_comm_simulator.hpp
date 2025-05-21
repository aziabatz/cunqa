#pragma once

#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/Operation.hpp"
#include "Definitions.hpp"

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"

#include "utils/json.hpp"


namespace cunqa {
namespace sim {


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
        targets = target_qubits;
        parameter = std::move(params);
    }

    [[nodiscard]] std::unique_ptr<Operation> clone() const override {
        return std::make_unique<DistributedClassicalCommunicationOperation>(*this);
    }

    // TODO
    void addControl(qc::Control c) override {
        std::cout << "Dummy method to override the pure virtual addControl()" << "\n";    
    }

    void clearControls() override { 
        std::cout << "Dummy method to override the pure virtual clearControls()" << "\n"; 
    }

    void removeControl(const qc::Control c) override {
        std::cout << "Dummy method to override the pure virtual removeControl()" << "\n"; 
    }

    qc::Controls::iterator removeControl(const qc::Controls::iterator it) override {
        return controls.erase(it); 
    }

    void dumpOpenQASM(std::ostream& of, const qc::RegisterNames& qreg, const qc::RegisterNames& creg, std::size_t indent, bool openQASM3) const override {
        std::cout << "Dummy method to override the pure virtual dumpOpenQASM()" << "\n";
    }

    void invert() override {
        std::cout << "Dummy method to override the pure virtual invert()" << "\n";
    }

protected:
    std::string sending_endpoint;
    std::string receiving_endpoint;
    qc::Qubit control_qubit;

};


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

    void saysomething()
    {
        std::cout << "Hola caracola" << "\n";
    }

    
    // Attributes
    bool has_classic_communications = false;
    JSON circuit;
protected:
    

};


class DistributedCircuitSimulator : public CircuitSimulator<dd::DDPackageConfig>
{
public:
    // Constructors
    DistributedCircuitSimulator() = default;
    DistributedCircuitSimulator(std::unique_ptr<QuantumComputation>&& qc_) : CircuitSimulator(std::unique_ptr<qc::QuantumComputation>(std::move(qc_)))
    {}

    // Methods
    std::map<std::size_t, bool> singleShot(bool ignoreNonUnitaries) override;

/*     void sayhello()
    {
        this->qc->saysomething();
    }
 */
};
    

class MunichCCSimulator final : public SimulatorStrategy<SimpleBackend> {
public:
    MunichCCSimulator() = default;
    ~MunichCCSimulator() = default;

    inline std::string get_name() const override {return "MunichSimulator";}
    JSON execute(const SimpleBackend& backend, const QuantumTask& circuit) override;
};



} // End namespace sim
} // End namespace cunqa