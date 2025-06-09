#include "cunqa_classical_comm_simulator.hpp"
#include "cunqa_executors.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {

JSON CunqaCCSimulator::execute(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the execute() method of CCCunqa.");
    return cunqa_execution_<ClassicalCommBackend>(backend, quantum_task, this->classical_channel.get());
}

std::string CunqaCCSimulator::get_communication_endpoint_()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}

} // End namespace sim
} // End namespace cunqa