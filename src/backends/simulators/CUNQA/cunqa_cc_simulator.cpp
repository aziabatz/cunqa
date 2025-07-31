#include "cunqa_cc_simulator.hpp"
#include "cunqa_adapters/cunqa_computation_adapter.hpp"
#include "cunqa_adapters/cunqa_simulator_adapter.hpp"

namespace cunqa {
namespace sim {

CunqaCCSimulator::CunqaCCSimulator()
{
    classical_channel.publish();
}

JSON CunqaCCSimulator::execute([[maybe_unused]] const CCBackend& backend, const QuantumTask& quantum_task)
{
    std::vector<std::string> connect_with = quantum_task.sending_to;
    classical_channel.connect(connect_with);

    CunqaComputationAdapter cunqa_ca(quantum_task);
    CunqaSimulatorAdapter cunqa_sa(cunqa_ca);

    return cunqa_sa.simulate(&backend, &classical_channel);
}


} // End namespace sim
} // End namespace cunqa