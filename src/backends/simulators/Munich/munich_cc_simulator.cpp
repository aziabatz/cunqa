#include <chrono>
#include <optional>

#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"

#include "munich_cc_simulator.hpp"

#include "utils/constants.hpp"

namespace cunqa {
namespace sim {


JSON MunichCCSimulator::execute(const CCBackend& backend, const QuantumTask& circuit)
{
    LOGGER_DEBUG("We are in the execute() method.");
    if (!circuit.is_distributed) {
        return this->usual_execution_(backend, circuit);
    } else {
        return this->distributed_execution_(backend, circuit);
    }   
} 


std::string MunichCCSimulator::_get_communication_endpoint()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}

} // End namespace sim
} // End namespace cunqa

