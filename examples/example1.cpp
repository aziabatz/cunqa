//#include "qpu.hpp"
#include "utils/json.hpp"
#include <iostream>
#include <thread>
#include <queue>
#include <chrono>
#include "comm/client.hpp"

cunqa::JSON backend_config = cunqa::JSON::parse(R"(
{
    "backend":
    {
        "name": "Example Backend",
        "version": "1.2.0",
        "simulator": "Aer",
        "n_qubits": 32,
        "url": "http://example-backend.com",
        "is_simulator": true,
        "conditional": false,
        "memory": true,
        "max_shots": 8192,
        "description": "This is an example backend configuration.",
        "basis_gates": "x",
        "custom_instructions": "custom_gate1"
    } 
}
)");

std::string circuit = R"(
{
    "config": {
        "shots": 1024,
        "method": "statevector",
        "memory_slots": 7
    },
    "instructions": [
    {
        "name": "h",
        "qubits": [0]
    },
    {
        "name": "cx",
        "qubits": [0, 1]
    },
    {
        "name": "measure",
        "qubits": [0],
        "memory": [0]
    },
    {
        "name": "measure",
        "qubits": [1],
        "memory": [1]
    }
    ]
}
)";

using namespace cunqa::comm;

int main(int argc, char *argv[])
{
    std::string mode(argv[1]);

    Client cliente{};
    cliente.connect(argv[2], argv[3]);
    cliente.send_circuit(circuit);
    
    auto result = cliente.recv_results();
    std::cout << result << "\n";
    
    return 0;
}