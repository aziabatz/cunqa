#pragma once

#include <unordered_map>
#include "aer_simulator.hpp"
#include "munich_simulator.hpp"

// Define Sim Types as a closed list
enum class SimType {
    Aer,
    Munich,
    AerNoise
};

const std::unordered_map<std::string, SimType> SIM_NAMES = {
    {"Aer", SimType::Aer},
    {"Munich", SimType::Munich},
    {"AerNoise", SimType::AerNoise}
};

template<SimType T, SimType U>
struct is_same : std::false_type {};
 
template<SimType T>
struct is_same<T, T> : std::true_type {};

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

template <>
struct SimClass<SimType::AerNoise> {
    using type = AerNoise;
};