#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <optional>
#include "zmq.hpp"
#include <nlohmann/json.hpp>

#include "utils/constants.hpp"
#include "config/net_config.hpp"
//#include "simulators/simulator.hpp"
#include "logger/logger.hpp"

using json = nlohmann::json;

std::string get_my_endpoint(const std::string_view& net = INFINIBAND) 
{
    auto id = std::getenv("SLURM_LOCALID");
    std::string ports = std::getenv("SLURM_STEP_RESV_PORTS");
    std::string port;
    SPDLOG_LOGGER_DEBUG(logger, "SLURM_STEP_RESV_PORTS: {}", ports);

    if(ports != "" && id != "")
    {
        size_t pos = ports.find('-');
        if (pos != std::string::npos) 
        {
            std::string base_port_str = ports.substr(pos + 1);
            int base_port = std::stoi(base_port_str);
            
            port = std::to_string(base_port - std::stoi(id));
            
        } else 
        {
            std::cerr << "Not a valid expression format of the ports.\n";
            return "-1"s;
        }
    } else 
    {
        std::cerr << "The required environment variables are not set (not in a SLURM job).\n";
        return "-1"s;
    }

    SPDLOG_LOGGER_DEBUG(logger, "Port selected: {}", port);

    std::unordered_map<std::string, std::string> IPs = get_IP_addresses();

    SPDLOG_LOGGER_DEBUG(logger, "IP addresses picked.");

    std::string endpoint = "tcp://" + IPs.at(std::string(net)) + ":" + port;

    SPDLOG_LOGGER_DEBUG(logger, "Endpoint created: {}.", endpoint);

    return endpoint;
}

// Returns the communication endpoints for all the QPUs except the current one
std::vector<std::string> get_others_endpoints(std::string& my_endpoint, const std::string_view& net = INFINIBAND)
{
    std::vector<std::string> others_endpoints;

    std::string cunqa_info_path = std::getenv("CUNQA_INFO_PATH");
    
    std::ifstream qpus_json_file(cunqa_info_path);
    if (!qpus_json_file) {
        SPDLOG_LOGGER_ERROR(logger, "Impossible to read the file {}", cunqa_info_path);
        return {};
    }

    json qpus_json;
    qpus_json_file >> qpus_json; 

    for (const auto& item : qpus_json) {
        if (item.at("comm_info").at("zmq") != my_endpoint) {
            others_endpoints.push_back(item.at("comm_info").at("zmq"));
        }
    }

    return others_endpoints;
}