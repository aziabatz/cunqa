#include "simulator_strategy.hpp"
#include "circuit.hpp"

namespace cunqa {
namespace sim {

class MunichSimpleSimulator final : SimulatorStrategy {
public:
    MunichSimpleSimulator() = default;
    ~MunichSimpleSimulator() = default;

    JSON execute(SimpleBackend backend, QuantumTask circuit);
}

} // End of sim namespace
} // End of cunqa namespace