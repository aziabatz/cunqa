#pragma once

#include "backend_config.hpp"
#include "net_config.hpp"
#include "utils/json.hpp"
#include "utils/helpers.hpp"

namespace config {

template <SimType sim_type = SimType::Aer>
class QPUConfig {
public:
    std::string mode;
    std::string family_name;
    std::string slurm_job_id;
    BackendConfig<sim_type> backend_config;
    NetConfig net_config;
    std::string filepath;

    QPUConfig(const std::string& mode, const cunqa::JSON& config, const std::string& filepath) :
        filepath{filepath}, family_name(config.at("family_name")), slurm_job_id(config.at("slurm_job_id"))
    {
        if (config.contains("backend"))
            this->backend_config = config.contains("noise") ? 
                           BackendConfig<sim_type>(config.at("backend"), config.at("noise")) : 
                           BackendConfig<sim_type>(config.at("backend"), cunqa::JSON());
        else
            this->backend_config = BackendConfig<sim_type>();
        
        if (config.contains("net"))
            this->net_config = NetConfig(config.at("net"));
        else {
            this->net_config = NetConfig::myNetConfig(mode);
        }
    }

    void set_backendconfig(cunqa::JSON backend_conf_json){
        this->backend_config = BackendConfig<sim_type>(backend_conf_json);
    }

    void set_netconfig(cunqa::JSON net_conf_json){
        this->net_config = NetConfig(net_conf_json);
    }
};

// ------------------------------------------------
// ----- Conversion functions for json format -----
// ------------------------------------------------
template <SimType sim_type>
void to_json(cunqa::JSON& j, const QPUConfig<sim_type>& qpu_config)
{
    std::string mode = qpu_config.mode;
    std::string family_name = qpu_config.family_name;
    std::string slurm_job_id = qpu_config.slurm_job_id;
    cunqa::JSON net = qpu_config.net_config;
    cunqa::JSON backend = qpu_config.backend_config;

    j = {   
            {"mode", mode},
            {"family_name", family_name},
            {"slurm_job_id", slurm_job_id},
            {"net", net}, 
            {"backend", backend}
        };
}

template <SimType sim_type>
void from_json(const cunqa::JSON& j, QPUConfig<sim_type>& qpu_config) 
{
    qpu_config.mode = j.at("mode");
    qpu_config.family_name = j.at("family_name");
    qpu_config.slurm_job_id = j.at("slurm_job_id");
    qpu_config.net_config = j.at("net").template get<NetConfig>();
    qpu_config.backend_config = j.at("backend").template get<BackendConfig<sim_type>>();
}; 

}