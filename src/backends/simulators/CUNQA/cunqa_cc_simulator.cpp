#include "cunqa_classical_comm_simulator.hpp"
#include "cunqa_executors.hpp"


using namespace std::string_literals;
namespace {
    const auto store = getenv("STORE");
    const std::string filepath = store + "/.cunqa/communications.json"s;
}

namespace cunqa {
namespace sim {

CunqaCCSimulator::CunqaCCSimulator() : classical_channel(std::make_unique<comm::ClassicalChannel>())
{
    JSON communications_endpoint = 
    {
        {"communications_endpoint", classical_channel->endpoint}
    };
    write_on_file(communications_endpoint, filepath);
}

JSON CunqaCCSimulator::execute(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the execute() method of CCCunqa.");
    return cunqa_execution_<ClassicalCommBackend>(backend, quantum_task, this->classical_channel.get());
}


} // End namespace sim
} // End namespace cunqa