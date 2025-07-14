#include "munich_qc_simulator.hpp"
#include "munich_adapters/circuit_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"


namespace cunqa {
namespace sim {

MunichQCSimulator::MunichQCSimulator()
{ 
    classical_channel.publish();
    auto executor_endpoint = classical_channel.recv_info("executor");
    LOGGER_DEBUG("Executor endpoint received: {}", executor_endpoint);
    classical_channel.connect(executor_endpoint, "executor");
};


JSON MunichQCSimulator::execute(const QCBackend& backend, const QuantumTask& quantum_task)
{
    auto circuit = to_string(quantum_task);
    classical_channel.send_info(circuit, "executor");
    if (circuit != "") {
        auto results = classical_channel.recv_info("executor");
        return JSON::parse(results);
    }
    return JSON();
        
}

} // End namespace sim
} // End namespace cunqa

