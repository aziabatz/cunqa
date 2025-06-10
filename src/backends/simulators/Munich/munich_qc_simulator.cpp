#include "munich_cc_simulator.hpp"

#include <chrono>
#include <optional>
#include <iostream>
#include <fstream>
#include <sys/file.h>
#include <unistd.h>

#include "utils/constants.hpp"
#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "ir/QuantumComputation.hpp"

#include "quantum_task.hpp"
#include "backends/simulators/simulator_strategy.hpp"
#include "munich_helpers.hpp"

#include "utils/json.hpp"

namespace cunqa {
namespace sim {

MunichQCSimulator::MunichQCSimulator(): 
{ 
    classical_channel.publish();
    classical_channel.recv_endpoint("executor");
};


JSON MunichQCSimulator::execute(const QCBackend& backend, const QuantumTask& circuit)
{
    this->classical_channel->send_info(to_string(circuit));
}

std::string MunichQCSimulator::_get_communication_endpoint()
{
    std::string endpoint = this->classical_channel->endpoint;
    return endpoint;
}

} // End namespace sim
} // End namespace cunqa

