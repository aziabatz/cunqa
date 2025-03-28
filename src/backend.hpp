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
    BackendConfig<sim_type> backend_config;
    std::optional<CunqaStateVector> backend_statevector;

    Backend();
    Backend(BackendConfig<sim_type> backend_config);

    inline json run(json& circuit_json);
    inline void initialize_statevector(int& n_qubits);
    inline void apply_gate(std::string& instruction_name, std::array<int, 3>& qubits, std::vector<double> param = {0.0});
    inline int apply_measure(std::string& instruction_name, std::array<int, 3>& qubits);
    inline int get_statevector_status();
    inline void _restart_statevector();
};

template <SimType sim_type>
Backend<sim_type>::Backend() : simulator{std::make_unique<typename SimClass<sim_type>::type>()},
backend_config{}
{}

template <SimType sim_type>
Backend<sim_type>::Backend(BackendConfig<sim_type> backend_config) : backend_config{backend_config}
{}

template <SimType sim_type>
inline json Backend<sim_type>::run(json& circuit_json)
{
    try {
        return SimClass<sim_type>::type::execute(circuit_json, this->backend_config.noise_model, config::RunConfig(circuit_json.at("config")));
        
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Error parsing the run configuration - {}", e.what());
        return {};
    }
} 

template <SimType sim_type>
inline void Backend<sim_type>::initialize_statevector(int& n_qubits)
{
    this->backend_statevector = CunqaStateVector(1 << n_qubits);
    this->backend_statevector.value()[0] = 1.0;
}

template <SimType sim_type>
inline void Backend<sim_type>::apply_gate(std::string& instruction_name, std::array<int, 3>& qubits, std::vector<double> param)
{
    try {
        this->backend_statevector.value() = SimClass<sim_type>::type::_apply_gate(instruction_name, this->backend_statevector.value(), qubits, param);
        
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Error applying gate");
    }
}

template <SimType sim_type>
inline int Backend<sim_type>::apply_measure(std::string& instruction_name, std::array<int, 3>& qubits)
{
    MeasurementOutput measurement;
    try {
        measurement = SimClass<sim_type>::type::_apply_measure(instruction_name, this->backend_statevector.value(), qubits);
        this->backend_statevector.value() = measurement.statevector;
        return measurement.measure;
        
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Error applying instruction");
        return -1;
    }
}

template <SimType sim_type>
inline void Backend<sim_type>::_restart_statevector()
{
    this->backend_statevector.value().assign(this->backend_statevector.value().size(), {0.0, 0.0});
    this->backend_statevector.value()[0] = 1.0;
}

template <SimType sim_type>
inline int Backend<sim_type>::get_statevector_status()
{
    int position;
    auto it = std::find_if(this->backend_statevector.value().begin(), this->backend_statevector.value().end(), [](const complex& c) {
        return c != std::complex<double>(0, 0); // Check for nonzero
    });

    position = std::distance(this->backend_statevector.value().begin(), it);

    return position;
}

