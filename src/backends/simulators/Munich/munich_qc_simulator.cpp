#include "munich_qc_simulator.hpp"

namespace cunqa {
namespace sim {

MunichQCSimulator::MunichQCSimulator()
{ 
    classical_channel.publish();
    auto executor_endpoint = classical_channel.recv_info("executor");
    classical_channel.connect(executor_endpoint, "executor");
};


JSON MunichQCSimulator::execute(const QCBackend& backend, const QuantumTask& circuit)
{
    classical_channel.send_info(to_string(circuit), "executor");
    auto results = classical_channel.recv_info("executor");

    if (results == "prueba")
        return {{"prueba", "OK"}};
    return JSON();
}

} // End namespace sim
} // End namespace cunqa

