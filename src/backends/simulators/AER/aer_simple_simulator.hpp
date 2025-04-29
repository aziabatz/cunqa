
#include "simulator_strategy.hpp"
#include "circuit.hpp"

namespace cunqa {
namespace sim {

class AerSimpleSimulator final : SimulatorStrategy {
public:
    AerSimpleSimulator() = default;
    ~AerSimpleSimulator() = default;

    JSON execute(SimpleBackend backend, QuantumTask circuit);
}

} // End of sim namespace
} // End of cunqa namespace