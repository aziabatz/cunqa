#include <iostream>
#include <fstream>
#include <memory>
#include "utils/constants.hpp"
#include "config/backend_config.hpp"
#include "simulators/simulator.hpp"

using json = nlohmann::json;
using namespace config;

template <SimType sim_type>
class Backend {
    std::unique_ptr<typename SimClass<sim_type>::type> simulator;
    BackendConfig<sim_type> backend_config; 
public:
    int backend_mpi_rank;

    Backend() :
        simulator{std::make_unique<typename SimClass<sim_type>::type>()},
        backend_config{}
    {}

    Backend(BackendConfig<sim_type> backend_config) :
        backend_config{backend_config}
    {} 

    json run(json& circuit_json) 
    {
        try {
            return SimClass<sim_type>::type::execute(circuit_json, backend_config.n_qubits, backend_config.noise_model, config::RunConfig(circuit_json.at("config")));
        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error parsing the run configuration - {}", e.what());
        }
        return {};
    }

};