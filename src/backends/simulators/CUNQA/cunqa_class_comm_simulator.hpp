#pragma once

#include <vector>
#include <string>

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {

class CunqaCCSimulator final : public SimulatorStrategy<SimpleBackend>
{
public:
    // Constructors
    CunqaCCSimulator() = default;
    ~CunqaCCSimulator() override;


    // Methods
    inline std::string get_name() override { return "CCCunqa"};
    cunqa::JSON execute(const SimpleBackend& backend, const QuantumTask& quantumtask) const override;

    // Attributes
};

} // End namespace sim
} // End namespace cunqa
