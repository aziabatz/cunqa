#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "quantum_computation_adapter.hpp"
#include "classical_channel.hpp"

#include "utils/json.hpp"


namespace cunqa {
namespace sim {

class CircuitSimulatorAdapter : public CircuitSimulator<dd::DDPackageConfig>
{
public:

    //QuantumComputationAdapter qca_;

    // Constructors
    CircuitSimulatorAdapter() = default;
    CircuitSimulatorAdapter(std::unique_ptr<QuantumComputationAdapter>&& qc_) : CircuitSimulator(std::unique_ptr<QuantumComputationAdapter>(std::move(qc_)))
    {}

    inline void initializeSimulationAdapter(std::size_t nQubits)
    {
        this->initializeSimulation(nQubits);
    }

    inline void applyOperationToStateAdapter(std::unique_ptr<qc::Operation>&& op)
    {
        this->applyOperationToState(op);
    }

    inline char measureAdapter(dd::Qubit i) 
    {
        return this->measure(i);
    }

    JSON simulate(std::size_t shots, comm::ClassicalChannel* classical_channel = nullptr); // TODO: override?
};

} // End of sim namespace
} // End of cunqa namespace