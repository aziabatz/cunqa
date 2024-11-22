#pragma once

#include <nlohmann/json.hpp>
#include <iostream>
#include "config.hpp"
#include "../utils/constants.hpp"

using namespace std::literals;
using json = nlohmann::json;

enum class Simulator {
    Aer,
    Munich
};

template<Simulator T, Simulator U>
struct is_same : std::false_type {};
 
template<Simulator T>
struct is_same<T, T> : std::true_type {};

namespace config {
namespace backend {

template <Simulator sim = Simulator::Aer>
class BackendConfig : Config {
public:
    std::string name;
    std::string version;
    int n_qubits;
    std::string url;
    bool simulator;
    bool conditional;
    bool memory;
    int max_shots;
    std::string description;
    std::optional<std::string> coupling_map;
    std::string basis_gates;
    std::string custom_instructions;
    std::vector<std::string> gates;

    template<Simulator T = sim,
             typename std::enable_if_t<is_same<T, Simulator::Aer>::value, bool> = true>
    BackendConfig() 
        : name{"AerSimulator"},
          version{}
    { }

    template<Simulator T = sim,
             typename std::enable_if_t<is_same<T, Simulator::Munich>::value, bool> = true>
    BackendConfig() 
        : name{"MunichSimulator"}
    { }

    template<Simulator T = sim,
             typename std::enable_if_t<is_same<T, Simulator::Aer>::value, bool> = true>
    BackendConfig(const json& config) 
        : name{"AerSimulator"}
    { }

    template<Simulator T = sim,
             typename std::enable_if_t<is_same<T, Simulator::Munich>::value, bool> = true>
    BackendConfig(const json& config) 
        : name{"MunichSimulator"}
    { }


    void load(const json& config) override {
        json
    }

    json dump() const override {
        return {};
    }
};  

void to_json(json& j, const BackendConfig<Simulator::Aer>& backend_conf)
{
    
    /* json ips{};

    for (const auto& net_bind : backend_conf.IPs) {
        ips[net_bind.first] = net_bind.second;
    }

    j = {   
            {"hostname", backend_conf.hostname}, 
            {"IPs", ips},
            {"port", backend_conf.port},
        }; */
}

// TODO: Generalizar esto
void from_json(const json& j, BackendConfig<Simulator::Aer>& NetConfig) 
{
    j.at("hostname").get_to(NetConfig.hostname);
    /* j.at("hostname").get_to(NetConfig.hostname);
    for (auto& netbind : j.at("IPs").items()) {
        NetConfig.IPs[netbind.key()] = netbind.value();
    }
    j.at("port").get_to(NetConfig.port); */
}; 

}
};

std::ostream& operator<<(std::ostream& os, const config::backend::BackendConfig<Simulator::Aer>& config) {
    /* os << "\nIPs: \n";
    for (const auto& net_bind : config.IPs) {
            os << net_bind.first << " ---> " << net_bind.second << "\n";
    }
    os << "\nPuerto: " << config.port
       << "\nHostname: " << config.hostname << "\n\n"; */
    return os;
}