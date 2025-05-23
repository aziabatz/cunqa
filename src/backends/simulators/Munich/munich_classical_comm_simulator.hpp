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
    

private:
    JSON usual_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);
    JSON distributed_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);
    std::string get_communication_endpoint_() override;

    std::unique_ptr<comm::ClassicalChannel> classical_channel;
    
};


} // End namespace sim
} // End namespace cunqa