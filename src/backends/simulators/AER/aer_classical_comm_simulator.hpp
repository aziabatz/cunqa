#pragma once

#include "quantum_task.hpp"
#include "backends/classical_comm_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "classical_channel.hpp"
#include "utils/json.hpp"

#include "logger.hpp"

namespace cunqa {
namespace sim {

class AerCCSimulator final : public SimulatorStrategy<ClassicalCommBackend> {
public:
    AerCCSimulator();
    ~AerCCSimulator() = default;

    inline std::string get_name() const override {return "AerSimulator";}
    JSON execute(const ClassicalCommBackend& backend, const QuantumTask& circuit) override;
    

private:
    std::unique_ptr<comm::ClassicalChannel> classical_channel;
};


} // End namespace sim
} // End namespace cunqa