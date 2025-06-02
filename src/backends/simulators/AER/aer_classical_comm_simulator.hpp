#pragma once

#include "quantum_task.hpp"
#include "backends/classical_comm_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "classical_channel.hpp"
#include "utils/json.hpp"

namespace cunqa {
namespace sim {

class AerCCSimulator final : public SimulatorStrategy<ClassicalCommBackend> {
public:
    AerCCSimulator(): classical_channel(std::make_unique<comm::ClassicalChannel>()) {};
    ~AerCCSimulator() = default;

    inline std::string get_name() const override {return "AerSimulator";}
    JSON execute(const ClassicalCommBackend& backend, const QuantumTask& circuit) override;
    

private:
    JSON distributed_execution_(const ClassicalCommBackend& backend, const QuantumTask& quantum_task);
    std::string get_communication_endpoint_() override;

    std::unique_ptr<comm::ClassicalChannel> classical_channel;
    
};


} // End namespace sim
} // End namespace cunqa