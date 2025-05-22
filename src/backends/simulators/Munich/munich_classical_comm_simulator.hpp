#pragma once

#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/operations/Operation.hpp"
#include "Definitions.hpp"

#include "quantum_task.hpp"
#include "backends/classical_comm_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "classical_channel.hpp"

#include "utils/json.hpp"


namespace cunqa {
namespace sim {

class MunichCCSimulator final : public SimulatorStrategy<ClassicalCommBackend> {
public:
    MunichCCSimulator(): classical_channel(std::make_unique<comm::ClassicalChannel>()) {};
    ~MunichCCSimulator() = default;

    inline std::string get_name() const override {return "MunichSimulator";}
    JSON execute(const ClassicalCommBackend& backend, const QuantumTask& circuit) override;
    std::string _get_communication_endpoint() override;

    std::unique_ptr<comm::ClassicalChannel> classical_channel;
};

// Free functions
JSON usual_execution(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);
JSON distributed_execution(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);


// New operation for Measure and Send





enum OpType : std::uint8_t {
    MeasureAndSend,
    RemoteCIf
};


class MeasureAndSend : public qc::Operation
{
public:
    MeasureAndSend() = default;
    MeasureAndSend(std::string& sending_endpoint, qc::Qubit& control_qubit): sending_endpoint{sending_endpoint}, control_qubit{control_qubit}
    {
        //type = static_cast<qc::OpType>(cunqa::sim::OpType::MeasureAndSend);
        //cunqa_type = qt;
    }

    // Methods
    bool isNonUnitaryOperation() const override { return true; }
    //qc::OpType getType() const override { return cunqa_type; }

    [[nodiscard]] std::unique_ptr<Operation> clone() const override {
        return std::make_unique<MeasureAndSend>(*this);
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
    qc::Qubit control_qubit;
    //OpType cunqa_type;

};

// New operation for Remote Controlled Gate
class RemoteCIf : public qc::Operation
{
public:
    RemoteCIf() = default;
    RemoteCIf(const qc::OpType op, std::string& receiving_endpoint, qc::Targets& target_qubits, const std::vector<qc::fp>& params = {}):receiving_endpoint{receiving_endpoint}, target_qubits{target_qubits}, params{params}
    {
        //type = static_cast<qc::OpType>(cunqa::sim::OpType::RemoteCIf);
        //cunqa_type = qt;
    }

    // Methods
    bool isClassicControlledOperation() const override { return true; }
    //qc::OpType getType() const override { return cunqa_type; }

    [[nodiscard]] std::unique_ptr<Operation> clone() const override {
        return std::make_unique<RemoteCIf>(*this);
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
    std::string receiving_endpoint;
    qc::Targets target_qubits;
    std::vector<qc::fp> params;
    //OpType cunqa_type;

};


// Extension of qc::QuantumComputation for Distributed Classical Communications
class ClassicalCommQuantumComputation : public qc::QuantumComputation
{
public:
    // Constructors
    ClassicalCommQuantumComputation() = default;
    ClassicalCommQuantumComputation(std::string& qasm_qc) : qc::QuantumComputation(qc::QuantumComputation::fromQASM(qasm_qc)) {}
    ClassicalCommQuantumComputation(const QuantumTask& quantum_task) : qc::QuantumComputation(quantum_task.config.at("num_qubits").get<std::size_t>(), quantum_task.config.at("num_clbits").get<std::size_t>()), circuit(quantum_task.circuit) {}

    // Methods
    void measure_and_send(std::string& sending_endpoint, qc::Qubit& control_qubit);
    void remote_c_if(const qc::OpType op, std::string& receiving_endpoint, qc::Targets& target_qubits, const std::vector<qc::fp>& params = {});

    void set_circuit();
    
    // Attributes
    bool has_classic_communications = false;
    std::vector<JSON> circuit;
protected:
    
};


class ClassicalCommCircuitSimulator : public CircuitSimulator<dd::DDPackageConfig>
{
public:
    // Constructors
    ClassicalCommCircuitSimulator() = default;
    ClassicalCommCircuitSimulator(std::unique_ptr<ClassicalCommQuantumComputation>&& qc_): CircuitSimulator(std::unique_ptr<ClassicalCommQuantumComputation>(std::move(qc_)))
    {}

    // Methods
    std::map<std::size_t, bool> CCsingleShot();

    void CCinitializeSimulation(std::size_t nQubits)
    {
        this->initializeSimulation(nQubits);
    }

    void CCapplyOperationToState(std::unique_ptr<qc::Operation>& op)
    {
        this->applyOperationToState(op);
    }

    char CCmeasure(dd::Qubit i) 
    {
        return this->measure(i);
    }


};
    



} // End namespace sim
} // End namespace cunqa