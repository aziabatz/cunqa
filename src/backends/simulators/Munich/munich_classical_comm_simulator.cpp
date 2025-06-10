#include "munich_classical_comm_simulator.hpp"
#include "munich_executors.hpp"
#include "munich_helpers.hpp"

#include <chrono>
#include <optional>

#include "quantum_task.hpp"
#include "backends/simulators/simulator_strategy.hpp"


using namespace std::string_literals;
namespace {
    const auto store = getenv("STORE");
    const std::string filepath = store + "/.cunqa/communications.json"s;
}

namespace cunqa {
namespace sim {

MunichCCSimulator::MunichCCSimulator() : classical_channel(std::make_unique<comm::ClassicalChannel>()) 
{
    JSON communications_endpoint = 
    {
        {"communications_endpoint", classical_channel->endpoint}
    };
    write_on_file(communications_endpoint, filepath);
}

JSON MunichCCSimulator::execute(const ClassicalCommBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the execute() method of CCMunich.");
    if (!quantum_task.is_dynamic) {
        return usual_execution_<ClassicalCommBackend>(backend, quantum_task);
    } else {
        return dynamic_execution_<ClassicalCommBackend>(backend, quantum_task, this->classical_channel.get());
    }   
}

} // End namespace sim
} // End namespace cunqa

