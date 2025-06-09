#pragma once

#include "quantum_task.hpp"
#include "backends/classical_comm_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "classical_channel.hpp"
#include "utils/json.hpp"


namespace cunqa {
namespace sim {

class CunqaCCSimulator final : public SimulatorStrategy<ClassicalCommBackend>
{
public:
    CunqaCCSimulator() : classical_channel(std::make_unique<comm::ClassicalChannel>())
    {}
    ~CunqaCCSimulator() = default;

    inline std::string get_name() const override {return "CunqaSimulator";}
    JSON execute(const ClassicalCommBackend& backend, const QuantumTask& quantumtask) override; 

private:
    std::string get_communication_endpoint_() override;

    std::unique_ptr<comm::ClassicalChannel> classical_channel;
};

} // End namespace sim
} // End namespace cunqa
