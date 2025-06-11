#pragma once

#include <chrono>
#include <optional>

#include "quantum_task.hpp"
#include "backends/qc_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "classical_channel.hpp"

#include "utils/json.hpp"


namespace cunqa {
namespace sim {

class MunichQCSimulator final : public SimulatorStrategy<QCBackend> {
public:
    MunichQCSimulator();
    ~MunichQCSimulator() = default;

    inline std::string get_name() const override {return "MunichQCSimulator";}
    JSON execute(const QCBackend& backend, const QuantumTask& circuit) override;

private:
    comm::ClassicalChannel classical_channel;

    JSON usual_execution_(const QCBackend& backend, const QuantumTask& quantum_task);
    JSON distributed_execution_(const QCBackend& backend, const QuantumTask& quantum_task);
};

} // End namespace sim
} // End namespace cunqa