#pragma once

#include <vector>

#include "quantum_task.hpp"
#include "classical_channel/classical_channel.hpp"
#include "backends/simple_backend.hpp"
#include "aer_computation_adapter.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {

class AerSimulatorAdapter
{
public:
    AerSimulatorAdapter() = default;
    AerSimulatorAdapter(AerComputationAdapter& qc) : qc{qc} {}

    JSON simulate(const SimpleBackend& backend);
    JSON simulate(comm::ClassicalChannel* classical_channel = nullptr);

    AerComputationAdapter qc;

private:
    std::string execute_shot_(const std::vector<QuantumTask>& quantum_tasks, comm::ClassicalChannel* classical_channel);
};


} // End of sim namespace
} // End of cunqa namespace