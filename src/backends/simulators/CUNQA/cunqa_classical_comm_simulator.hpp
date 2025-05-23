#pragma once

#include <vector>
#include <string>

#include "quantum_task.hpp"
#include "backends/classical_comm_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "utils/json.hpp"
#include "classical_channel.hpp"
#include "logger.hpp"

#include "src/executor.hpp"

namespace cunqa {
namespace sim {

class CunqaCCSimulator final : public SimulatorStrategy<ClassicalCommBackend>
{
public:
    // Constructors
    CunqaCCSimulator() : classical_channel(std::make_unique<comm::ClassicalChannel>())
    {
        LOGGER_DEBUG("CunqaCCSimulator instantiated.");
    }
    ~CunqaCCSimulator() override;


    // Methods
    inline std::string get_name() const override {return "CunqaSimulator";}
    JSON execute(const ClassicalCommBackend& backend, const QuantumTask& quantumtask) override; 

private:
    std::string get_communication_endpoint_() override;

    // Attributes
    std::unique_ptr<Executor> executor;
    std::unique_ptr<comm::ClassicalChannel> classical_channel;
};

} // End namespace sim
} // End namespace cunqa
