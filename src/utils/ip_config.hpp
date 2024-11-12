#pragma once

#include <nlohmann/json.hpp>
#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#ifndef __USE_POSIX
    #define __USE_POSIX
#endif
#include <limits.h>

using namespace std::literals;
using json = nlohmann::json;

namespace ip_config {

class IPConfig {
public:
    int qpu_id;
    std::string hostname;
    std::unordered_map<std::string, std::string> IPs;
    std::string port;

    IPConfig(const int& qpu_id)
        : qpu_id{qpu_id},
          hostname{_get_hostname()},
          IPs{_get_IP_addresses()},
          port{_get_port()} { }

    IPConfig(const int& qpu_id, const std::string& hostname)
        : qpu_id{qpu_id},
          hostname{hostname},
          IPs{_get_IP_addresses()},
          port{_get_port()} { }
    

    IPConfig& operator=(const IPConfig& other) 
    {
        if (this != &other) {
            hostname = other.hostname;
            IPs = other.IPs;
            port = other.port;
        }
        return *this;
    }

private:

    std::string _get_hostname(){
        char hostname[HOST_NAME_MAX];
        gethostname(hostname, sizeof(hostname));
        return hostname;
    }

    std::unordered_map<std::string, std::string> _get_IP_addresses() 
    {
        std::unordered_map<std::string, std::string> ips;
        struct ifaddrs *interfaces, *ifa;

        if (getifaddrs(&interfaces) == -1) {
            std::cerr << "Error getting the network interfaces\n";
            return std::unordered_map<std::string, std::string>();
        }

        for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;

            int family = ifa->ifa_addr->sa_family;
            if (family == AF_INET) {
                char ip[INET6_ADDRSTRLEN];
                void *addr;
                
                addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                inet_ntop(family, addr, ip, sizeof(ip));

                ips[ifa->ifa_name] = ip;
            }
        }

        freeifaddrs(interfaces);

        return ips;
    }


    // TODO: Hacer que el get_port no esté tan ligado a SLURM 
    std::string _get_port() 
    {
        auto id = std::stoi(std::getenv("SLURM_LOCALID"));
        std::string ports = std::getenv("SLURM_STEP_RESV_PORTS");

        if(ports != "")
        {
            size_t pos = ports.find('-');
            if (pos != std::string::npos) 
            {
                std::string base_port_str = ports.substr(0, pos);
                int base_port = std::stoi(base_port_str);
                
                return std::to_string(base_port + id);
                
            } else 
            {
                std::cerr << "Not a valid expression format of the ports.\n";
                return "-1"s;
            }
        } else 
        {
            std::cerr << "The required environment variables are not set (not).\n";
            return "-1"s;
        }
    }

};


void to_json(json& j, const IPConfig& ip_config)
    {
        json aux{};

        for (const auto& net_bind : ip_config.IPs) {
            aux[net_bind.first] = net_bind.second;
        }

        j = {   
                {"hostname", ip_config.hostname}, 
                {"IPs", aux},
                {"port", ip_config.port},
            };
    }
};



std::ostream& operator<<(std::ostream& os, const ip_config::IPConfig& config) {
    os << "\nIPs: \n";
    for (const auto& net_bind : config.IPs) {
            os << net_bind.first << " ---> " << net_bind.second << "\n";
    }
    os << "\nPuerto: " << config.port
       << "\nHostname: " << config.hostname << "\n\n";
    return os;
}