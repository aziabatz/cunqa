#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "utils/constants.hpp"
#include "utils/helpers/net_functions.hpp"
#include "logger.hpp"
#include "utils/json.hpp"

namespace cunqa {
namespace comm {

std::string get_my_endpoint() 
{
    auto id = std::getenv("SLURM_LOCALID");
    std::string ports = std::getenv("SLURM_STEP_RESV_PORTS");
    std::string port;
    LOGGER_DEBUG("SLURM_STEP_RESV_PORTS: {}", ports);

    if(ports != "" && id != "")
    {
        size_t pos = ports.find('-');
        if (pos != std::string::npos) 
        {
            std::string base_port_str = ports.substr(pos + 1);
            int base_port = std::stoi(base_port_str);
            
            port = std::to_string(base_port - std::stoi(id));
            
        } else {
            std::cerr << "Not a valid expression format of the ports.\n";
            return "-1";
        }
    } else {
        std::cerr << "The required environment variables are not set (not in a SLURM job).\n";
        return "-1";
    }

    LOGGER_DEBUG("Port selected: {}", port);

    std::string IP = get_global_IP_address();

    LOGGER_DEBUG("IP address picked.");

    std::string endpoint = "tcp://" + IP + ":" + port;

    LOGGER_DEBUG("Endpoint created: {}.", endpoint);

    return endpoint;
}

std::vector<std::string> get_others_endpoints(std::string& my_endpoint)
{
    std::vector<std::string> others_endpoints;

    std::string cunqa_info_path = std::getenv("CUNQA_INFO_PATH");
    
    std::ifstream qpus_json_file(cunqa_info_path);
    if (!qpus_json_file) {
        LOGGER_ERROR("Impossible to read the file {}", cunqa_info_path);
        return {};
    }

    JSON qpus_json;
    qpus_json_file >> qpus_json; 

    for (auto& item : qpus_json) {
        std::string aux_endpoint =  item.at("communications_endpoint").get<std::string>();
        if (aux_endpoint != my_endpoint) {
            others_endpoints.push_back(aux_endpoint);
        }
    }

    return others_endpoints;
}

} // End of comm namespace
} // End of cunqa namespace
