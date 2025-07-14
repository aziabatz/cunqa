#pragma once

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "utils/json.hpp"

#include "logger.hpp"

namespace cunqa {
namespace sim {

class CunqaSimpleSimulator final : public SimulatorStrategy<SimpleBackend>
{
public:
    CunqaSimpleSimulator() = default;
    ~CunqaSimpleSimulator() = default;

    inline std::string get_name() const override {return "CunqaSimulator";}
    JSON execute(const SimpleBackend& backend, const QuantumTask& quantumtask) override;

};

} // End namespace sim
} // End namespace cunqa
