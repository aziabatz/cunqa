
#include <string>
#include <optional>

#include "simulators/circuit_executor.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"

#include "utils/json.hpp"
#include "logger/logger.hpp"

#include "aer_simple_simulator.hpp"

namespace cunqa {
namespace sim {

JSON AerSimpleSimulator::execute(SimpleBackend backend, QuantumTask quantum_task){
    try {
        //TODO: Maybe improve them to send several circuits at once
        QuantumTask circuit(quantum_task.circuit);
        std::vector<std::shared_ptr<QuantumTask>> circuits;
        circuits.push_back(std::make_shared<QuantumTask>(circuit));

        JSON run_config_json(quantum_task.config);
        run_config_json["seed_simulator"] = run_config.seed;
        Config aer_config(run_config_json);
        
        Noise::NoiseModel noise_model(noise_model_json);
        Result result = controller_execute<Controller>(circuits, noise_model, aer_config);
        return result.to_json();
    } catch (const std::exception& e) {
        // TODO: specify the circuit format in the docs.
        LOGGER_ERROR("Error executing the circuit in the AER simulator.\n\tTry checking the format of the circuit sent and/or of the noise model.");
        return {{"ERROR", "\"" + std::string(e.what()) + "\""}};
    }
    return {};
}

} // End of sim namespace
} // End of cunqa namespace