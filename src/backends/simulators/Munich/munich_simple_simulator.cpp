#include "munich_simple_simulator.hpp"
#include "munich_executors.hpp"
#include "munich_helpers.hpp"

#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

namespace cunqa {
namespace sim {

JSON MunichSimpleSimulator::execute(const SimpleBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the execute() method of SimpleMunich.");
    if (!quantum_task.is_dynamic) {
        return usual_execution_<SimpleBackend>(backend, quantum_task);
    } else {
        return dynamic_execution_<SimpleBackend>(backend, quantum_task);
    }   
} 


} // End of sim namespace
} // End of cunqa namespace


