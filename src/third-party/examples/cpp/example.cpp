#include "simulators/circuit_executor.hpp"
#include "framework/json.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"
#include <string>

using namespace std::literals;
using namespace AER;

int main(){

    json_t json_file = JSON::load("qiskit-aer-example.json");
    Circuit circuit(json_file);

    auto aer_default = json_file["config"].template get<Config>();
    Noise::NoiseModel noise_default;

    std::vector<std::shared_ptr<Circuit>> circuits;
    circuits.push_back(std::make_shared<Circuit>(circuit));

    //Result result = controller_execute<Controller>(circuits, noise_default, aer_default);

    //Controller controller{};
    //controller.set_config(aer_default);
    //Result result = controller.execute(circuits, noise_default, aer_default); 

    Result result = controller_execute<Controller>(circuits, noise_default, aer_default);

    for (auto &res : result.results) {
        std::cout << res.data.to_json().dump(4) << "\n";
    }

    json_t result_json = result.to_json();

    std::ofstream file("results.json");
    file << result_json; 

    return 0;
}