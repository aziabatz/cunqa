#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"

#include "munich_cc_simulator.hpp"

#include "utils/constants.hpp"

namespace cunqa {
namespace sim {

MunichCCSimulator::MunichCCSimulator()
{
    classical_channel.publish();
};

JSON MunichCCSimulator::execute(const CCBackend& backend, const QuantumTask& circuit)
{
    LOGGER_DEBUG("We are in the execute() method.");
    if (!circuit.is_distributed) {
        return this->usual_execution_(backend, circuit);
    } else {
        return this->distributed_execution_(backend, circuit);
    }   
}

} // End namespace sim
} // End namespace cunqa

