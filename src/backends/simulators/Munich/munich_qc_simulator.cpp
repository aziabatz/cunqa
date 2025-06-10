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

namespace{

void endpoint_to_file(const std::string& endpoint) 
{
    const char* store = std::getenv("STORE");
    std::string filename = std::string(store) + "/endpoints_" + std::getenv("SLURM_JOB_ID");
    int file = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
    if (file == -1) {
        std::cerr << "Error al abrir el archivo" << std::endl;
        return;
    }
    flock(file, LOCK_EX);

    std::string current_data;
    std::ofstream out(filename, std::ios::app);
    out << endpoint << "\n";
    out.close();

    flock(file, LOCK_UN);
    close(file);
}

}


namespace cunqa {
namespace sim {

MunichQCSimulator::MunichQCSimulator(): 
    classical_channel(std::make_unique<comm::ClassicalChannel>()) 
{ 
    endpoint_to_file(classical_channel.endpoint);
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

