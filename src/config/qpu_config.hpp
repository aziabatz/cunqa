#pragma once

#include "backend_config.hpp"
#include "net_config.hpp"
#include <nlohmann/json.hpp>
#include "../utils/helpers.hpp"

using json = nlohmann::json;

namespace config {

template <typename SimType>
class QPUConfig {
public:
    BackendConfig<SimType> backend_conf;
    NetConfig net_conf;

    QPUConfig(json config_json)
    {
        if (config_json.at("backend")){
            this->backend_conf = _def_backendconfig(config_json["backend"]);
        } else {
            //TODO: aquí ponemos el por defecto ???
            this->backend_conf = BackendConfig<SimType>();
        }
            
        this->net_conf = _def_netconfig();
    }

    void set_backendconfig(json backend_conf_json){
        this->backend_conf = _def_backendconfig(backend_conf_json);
    }

private:

    BackendConfig<SimType> _def_backendconfig(json config_json) {
        // TODO: define how to read the json
    }

    NetConfig _def_netconfig() {
        return config::ip::NetConfig.myNetConfig();
    }

};

}