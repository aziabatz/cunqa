#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class MunichSimulator {
public:
    static json execute(json circuit_json, json noise_model_json, const config::RunConfig& run_config) {
        std::cerr << "Yet to be implemented\n";
        return json();
    }
};