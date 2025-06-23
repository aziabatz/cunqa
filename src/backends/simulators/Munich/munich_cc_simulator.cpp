#include "munich_cc_simulator.hpp"
#include "munich_adapters/circuit_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"

#include <chrono>

#include "StochasticNoiseSimulator.hpp"

#include "utils/constants.hpp"

namespace cunqa {
namespace sim {

MunichCCSimulator::MunichCCSimulator()
{
    classical_channel.publish();
};

JSON MunichCCSimulator::execute(const CCBackend& backend, const QuantumTask& quantum_task)
{
    LOGGER_DEBUG("MunichCCSimulator::execute()");
    std::vector<std::string> connect_with = quantum_task.sending_to;
    classical_channel.connect(connect_with);
    LOGGER_DEBUG("Connected");

    auto p_qca = std::make_unique<QuantumComputationAdapter>(quantum_task);
    LOGGER_DEBUG("QuantumComputationAdapter ready");
    CircuitSimulatorAdapter csa(std::move(p_qca));
    LOGGER_DEBUG("CircuitSimulatorAdapter ready");

    return csa.simulate(quantum_task.config.at("shots").get<std::size_t>(), &classical_channel);
}

} // End namespace sim
} // End namespace cunqa

