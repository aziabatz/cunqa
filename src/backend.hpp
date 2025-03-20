#include <iostream>
#include <fstream>
#include <memory>
#include <optional>

#include "utils/constants.hpp"
#include "config/backend_config.hpp"
#include "simulators/simulator.hpp"
#include "comm/qpu_comm.hpp"


using json = nlohmann::json;
using namespace config;

template <SimType sim_type>
class Backend {
    std::unique_ptr<typename SimClass<sim_type>::type> simulator;
    
public:
    int backend_mpi_rank;

    BackendConfig<sim_type> backend_config;

    Backend() :
        simulator{std::make_unique<typename SimClass<sim_type>::type>()},
        backend_config{}
    {}

    Backend(BackendConfig<sim_type> backend_config) :
        backend_config{backend_config}
    {} 

    json run(json& circuit_json, std::optional<ZMQSockets> zmq_sockets) 
    {
        try {
            return SimClass<sim_type>::type::execute(circuit_json, backend_config.noise_model, config::RunConfig(circuit_json.at("config")), std::move(zmq_sockets));
            
        } catch (const std::exception& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error parsing the run configuration - {}", e.what());
        }
        return {};
    }

};