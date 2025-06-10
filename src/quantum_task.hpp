#pragma once

#include <vector>
#include <string>
#include "utils/json.hpp"

namespace cunqa {

class QuantumTask {
public:
    JSON circuit;
    JSON config;
    std::vector<std::string> sending_to;
    bool is_distributed;

    QuantumTask() : is_distributed{false} {};

    QuantumTask(const JSON& circuit, const JSON& config): circuit(circuit), config(config) { };

    void update_circuit(const std::string& quantum_task);
    
private:
    
    void update_params_(const std::vector<double> params);
};

std::string to_string(const QuantumTask& data)
{
    if (data.circuit.empty())
        return "";
    return data.config.dump(4) + data.circuit.dump(4); 
}

} // End of cunqa namespace