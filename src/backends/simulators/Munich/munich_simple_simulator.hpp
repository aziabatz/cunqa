#pragma once

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {

class MunichSimpleSimulator final : public SimulatorStrategy<SimpleBackend> {
public:
    MunichSimpleSimulator() = default;
    ~MunichSimpleSimulator() = default;

    inline std::string get_name() const override { return "SimpleMunich"; }
    JSON execute(const SimpleBackend& backend, const QuantumTask& circuit) override;
};

} // End of sim namespace
} // End of cunqa namespace