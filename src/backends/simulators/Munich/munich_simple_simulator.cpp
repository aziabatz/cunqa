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
    auto p_qca = std::make_unique<QuantumComputationAdapter>(quantum_task);
    CircuitSimulatorAdapter csa(std::move(p_qca));
    return csa.simulate(quantum_task.config.at("shots").get<std::size_t>());
} 


} // End of sim namespace
} // End of cunqa namespace


