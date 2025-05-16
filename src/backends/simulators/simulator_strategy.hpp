#pragma once

#include "quantum_task.hpp"
#include "utils/json.hpp"

namespace cunqa {
namespace sim {

template <typename T>
class SimulatorStrategy {
public:

    virtual ~SimulatorStrategy() {};

    virtual inline std::string get_name() const = 0;
    virtual JSON execute(const T& backend, const QuantumTask& circuit) = 0;
    virtual std::string get_communication_endpoint() {return "";};
};

} // End of sim namespace
} // End of cunqa namespace