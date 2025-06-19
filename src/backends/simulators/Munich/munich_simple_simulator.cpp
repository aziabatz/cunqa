#include "munich_simple_simulator.hpp"
#include "munich_adapters/circuit_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"

#include "munich_helpers.hpp"

#include <chrono>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

namespace cunqa {
namespace sim {

JSON MunichSimpleSimulator::execute(const SimpleBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("We are in the execute() method of SimpleMunich.");
    auto p_quantum_computation = std::make_unique<QuantumComputationAdapter>(quantum_task);
    CircuitSimulatorAdapter circsim(std::move(p_quantum_computation));
    return circsim.simulate(quantum_task.config.at("shots").get<std::size_t>());
} 


} // End of sim namespace
} // End of cunqa namespace


