
#include "ir/QuantumComputation.hpp"

namespace cunqa {
namespace sim {

// Extension of qc::QuantumComputation for Distributed Classical Communications
class QuantumComputationAdapter : public qc::QuantumComputation
{
public:
    // Constructors
    QuantumComputationAdapter() = default;
    QuantumComputationAdapter(const QuantumTask& quantum_task) : qc::QuantumComputation(quantum_task.config.at("num_qubits").get<std::size_t>(), quantum_task.config.at("num_clbits").get<std::size_t>()) {}
    
};

} // End of sim namespace
} // End of cunqa namespace