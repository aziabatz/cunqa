#pragma once

#include <vector>
#include "utils/json.hpp"

namespace cunqa {

class QuantumTask {
public:
    JSON circuit;
    JSON config;

    QuantumTask() : is_parametric_{false} {};

    QuantumTask(const JSON& circuit,const JSON& config): circuit(circuit), config(config) { };

    void update_circuit(const std::string& quantum_task);

private:
    bool is_parametric_;
    
    void update_params_(const std::vector<double> params);
};

} // End of cunqa namespace