#pragma once 

#include "quantum_task.hpp"

#include "ir/QuantumComputation.hpp"

namespace cunqa {
namespace sim {

// Extension of qc::QuantumComputation for Distributed Classical Communications
class QuantumComputationAdapter : public qc::QuantumComputation
{
public:
    // Constructors
    QuantumComputationAdapter() = default;
    QuantumComputationAdapter(const QuantumTask& quantum_task) : 
        qc::QuantumComputation(quantum_task.config.at("num_qubits").get<std::size_t>(), quantum_task.config.at("num_clbits").get<std::size_t>()),
        quantum_tasks{quantum_task}
    {}
    QuantumComputationAdapter(const std::vector<QuantumTask>& quantum_tasks) : quantum_tasks{quantum_tasks}
    {
        size_t num_qubits = 0, num_clbits = 0;
        for(const auto& quantum_task: quantum_tasks) {
            num_qubits += quantum_task.config.at("num_qubits").get<std::size_t>();
            num_clbits += quantum_task.config.at("num_clbits").get<std::size_t>();
        }
        qc::QuantumComputation(num_qubits, num_clbits);
    }

    std::vector<QuantumTask> quantum_tasks;
};

} // End of sim namespace
} // End of cunqa namespace