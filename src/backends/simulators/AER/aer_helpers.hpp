#pragma once

#include "quantum_task.hpp"
#include "utils/json.hpp"

namespace cunqa {
namespace sim {

inline JSON quantum_task_to_AER(QuantumTask quantum_task) 
{
    return {{"instructions", quantum_task.circuit}};
}

} // End of sim namespace
} // End of cunqa namespace