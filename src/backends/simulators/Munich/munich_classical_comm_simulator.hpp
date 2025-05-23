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

private:
    JSON usual_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);
    JSON distributed_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);
};



// Extension of qc::QuantumComputation for Distributed Classical Communications
class ClassicalCommQuantumComputation : public qc::QuantumComputation
{
public:
    // Constructors
    ClassicalCommQuantumComputation() = default;
    ClassicalCommQuantumComputation(const QuantumTask& quantum_task) : qc::QuantumComputation(quantum_task.config.at("num_qubits").get<std::size_t>(), quantum_task.config.at("num_clbits").get<std::size_t>()) {}
    
};


// Extension of CircuitSimulator for Distributed Classical Communications
class ClassicalCommCircuitSimulator : public CircuitSimulator<dd::DDPackageConfig>
{
public:
    // Constructors
    ClassicalCommCircuitSimulator() = default;
    ClassicalCommCircuitSimulator(std::unique_ptr<ClassicalCommQuantumComputation>&& qc_): CircuitSimulator(std::unique_ptr<ClassicalCommQuantumComputation>(std::move(qc_)))
    {}

    // Methods
    std::map<std::size_t, bool> CCsingleShot();

    void CCinitializeSimulation(std::size_t nQubits) { this->initializeSimulation(nQubits); }

    void CCapplyOperationToState(std::unique_ptr<qc::Operation>& op) { this->applyOperationToState(op); }

    char CCmeasure(dd::Qubit i) { return this->measure(i); }

};
    


} // End namespace sim
} // End namespace cunqa