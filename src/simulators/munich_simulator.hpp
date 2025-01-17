#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class MunichSimulator {
public:

    MunichSimulator(const std::string& filepath) {}

    json execute(json circuit_json, const config::RunConfig& run_config) {
        std::cerr << "Yet to be implemented\n";
        return json();
    }
};