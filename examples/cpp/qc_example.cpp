#include "utils/json.hpp"
#include <iostream>
#include "comm/client.hpp"
#include <string>

std::string circuit = R"(
{
    "config": {
        "shots": 1024,
        "method": "statevector",
        "num_clbits": 2,
        "num_qubits": 2
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

using namespace std::string_literals;
using namespace cunqa::comm;

cunqa::JSON read_file(const std::string& filename)
{
    std::ifstream in(filename);

    if (!in.is_open()) {
        throw std::runtime_error("Error opening the communications file.");
    }

    cunqa::JSON j;
    if (in.peek() != std::ifstream::traits_type::eof())
        in >> j;
    in.close();
    return j;
}


int main(int argc, char *argv[ ])
{
    
    const auto store = getenv("STORE");
    const std::string filepath = store + "/.cunqa/qpus.json"s;
    cunqa::JSON qpus = read_file(filepath);

    std::cout << "Aqui\n";
    std::vector<Client> clients(2);
    int i=0;
    for (const auto& qpu: qpus) {
        clients[i].connect(qpu.at("net").at("ip"), qpu.at("net").at("port"));
        clients[i].send_circuit(circuit);
        i++;
    }
    std::cout << "Aqui\n";
    for (auto& client: clients) {
        std::cout << client.recv_results() << "\n";
    }  
    
    return 0;
}