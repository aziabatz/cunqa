#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include "zmq.hpp"

#include "utils/constants.hpp"
#include "config/net_config.hpp"
//#include "simulators/simulator.hpp"

#include "logger/logger.hpp"

std::string get_zmq_endpoint(const std::string_view& net = INFINIBAND) 
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

    LOGGER_DEBUG("Port selected: {}", port);

    std::unordered_map<std::string, std::string> IPs = get_IP_addresses();

    LOGGER_DEBUG("IP addresses picked.");

    std::string endpoint = "tcp://" + IPs.at(std::string(net)) + ":" + port;

    LOGGER_DEBUG("Endpoint created: {}.", endpoint);

    return endpoint;
}

