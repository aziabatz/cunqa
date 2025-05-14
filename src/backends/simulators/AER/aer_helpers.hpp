#pragma once

#include <regex>
#include <string>
#include "quantum_task.hpp"
#include "utils/json.hpp"

#include "logger.hpp"

namespace cunqa {
namespace sim {

QuantumTask quantum_task_to_AER(const QuantumTask& quantum_task)
{
    JSON new_config = {
        {"method", quantum_task.config.at("method")},
        {"shots", quantum_task.config.at("shots")},
        {"memory_slots", quantum_task.config.at("num_clbits")}
        // TODO: Tune in the different options of the AER simulator
    };

    //JSON Object because if not it generates an array
    JSON new_circuit = {
        {"config", new_config},
        {"instructions", JSON::parse(std::regex_replace(quantum_task.circuit.dump(),
                       std::regex("clbits"), "memory"))}
    };

    LOGGER_DEBUG("Circuito ANTES: {}", new_circuit.dump(4));

    return QuantumTask(new_circuit, new_config);
}

} // End of sim namespace
} // End of cunqa namespace