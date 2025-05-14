#pragma once

#include <vector>
#include <string>

#include "quantum_task.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "utils/json.hpp"
#include "classical_channel.hpp"

#include "src/executor.hpp"

namespace cunqa {
namespace sim {

class CunqaCCSimulator final : public SimulatorStrategy<SimpleBackend>
{
public:
    // Constructors
    CunqaCCSimulator() = default;
    ~CunqaCCSimulator() override;


    // Methods
    inline std::string get_name() const override { return "CunqaClassicalCommunications";};
    JSON execute(const SimpleBackend& backend, const QuantumTask& quantumtask) override; 

    // Attributes
    std::unique_ptr<Executor> executor;
    std::unique_ptr<comm::ClassicalChannel> classical_channel;
};

} // End namespace sim
} // End namespace cunqa
