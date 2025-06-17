#include <chrono>
#include <optional>

#include "StochasticNoiseSimulator.hpp"

#include "munich_cc_simulator.hpp"
#include "munich_adapters/circuit_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"

#include "utils/constants.hpp"

namespace cunqa {
namespace sim {

MunichCCSimulator::MunichCCSimulator() : classical_channel{std::make_unique<comm::ClassicalChannel>()}
{
    classical_channel->publish();
};

JSON MunichCCSimulator::execute(const CCBackend& backend, const QuantumTask& circuit)
{
    LOGGER_DEBUG("MunichCCSimulator::execute()");
    std::vector<std::string> connect_with = circuit.sending_to;
    classical_channel->connect(connect_with);
    LOGGER_DEBUG("Connected");

    std::size_t shots = circuit.config.at("shots").get<std::size_t>();

    std::unique_ptr<QuantumComputationAdapter> quantum_computation = std::make_unique<QuantumComputationAdapter>(circuit);
    LOGGER_DEBUG("QuantumComputationAdapter ready");
    CircuitSimulatorAdapter circuit_simulator(std::move(quantum_computation));
    LOGGER_DEBUG("CircuitSimulatorAdapter ready");

    JSON result = circuit_simulator.simulate(shots, std::move(classical_channel));
    LOGGER_DEBUG("Result: {}", result.dump());
    return result;
}

} // End namespace sim
} // End namespace cunqa

