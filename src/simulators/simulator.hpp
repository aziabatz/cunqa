#pragma once

#include <unordered_map>
#include "aer_simulator.hpp"
#include "munich_simulator.hpp"

// Define Sim Types as a closed list
enum class SimType {
    Aer,
    Munich
};

const std::unordered_map<std::string, SimType> SIM_NAMES = {
    {"Aer", SimType::Aer},
    {"Munich", SimType::Munich}
};

// Depending on the SimType selected, a Simulator class is selected
template <SimType = SimType::Aer>
struct SimClass {
    using type = AerSimulator;
};

template <>
struct SimClass<SimType::Aer> {
    using type = AerSimulator;
};

template <>
struct SimClass<SimType::Munich> {
    using type = MunichSimulator;
};