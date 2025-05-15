#pragma once

#include <vector>
#include <string>

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "utils/json.hpp"

#include "src/executor.hpp"

namespace cunqa {
namespace sim {

class CunqaSimpleSimulator final : public SimulatorStrategy<SimpleBackend>
{
public:
    // Constructors
    CunqaSimpleSimulator() = default;
    ~CunqaSimpleSimulator() override;


    // Methods
    inline std::string get_name() const override { return "CunqaSimulator";};
    JSON execute(const SimpleBackend& backend, const QuantumTask& quantumtask) override;

    // Attributes
    std::unique_ptr<Executor> executor;
};

} // End namespace sim
} // End namespace cunqa
