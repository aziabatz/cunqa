#pragma once

#include "simulators/circuit_executor.hpp"
#include "framework/json.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"
#include <string>

using json = nlohmann::json;
using namespace std::literals;
using namespace AER;

class AerSimulator {
public:

    static json execute_circuit(json circuit_json, json noise_model_json, json config_json) {

        Circuit circuit(circuit_json);
        Noise::NoiseModel noise_default;
        Config aer_default(config_json);

        std::vector<std::shared_ptr<Circuit>> circuits;
        circuits.push_back(std::make_shared<Circuit>(circuit));

        Result result = controller_execute<Controller>(circuits, noise_default, aer_default);

        return result.to_json();
    }

};