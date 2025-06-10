
#include "CircuitSimulator.hpp"
#include "StochasticNoiseSimulator.hpp"

namespace cunqa {
namespace sim {

class CircuitSimulatorAdapter : public CircuitSimulator<dd::DDPackageConfig>
{
public:
    // Constructors
    CircuitSimulatorAdapter() = default;
    CircuitSimulatorAdapter(std::unique_ptr<QuantumComputationAdapter>&& qc_): CircuitSimulator(std::unique_ptr<QuantumComputationAdapter>(std::move(qc_)))
    {}

    inline void initializeSimulationAdapter(std::size_t nQubits)
    {
        this->initializeSimulation(nQubits);
    }

    inline void applyOperationToStateAdapter(std::unique_ptr<qc::Operation>& op)
    {
        this->applyOperationToState(op);
    }

    inline char measureAdapter(dd::Qubit i) 
    {
        return this->measure(i);
    }

    std::map<std::string, std::size_t> simulate(std::size_t shots) override;
};

} // End of sim namespace
} // End of cunqa namespace