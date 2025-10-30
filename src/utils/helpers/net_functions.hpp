#pragma once

#include <iostream>
#include <sys/types.h>
#include <string>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

#include "logger.hpp"
#include "utils/constants.hpp"
#include "utils/helpers/runtime_env.hpp"

using namespace std::string_literals;

// Secure cast of size
template<typename TO, typename FROM>
TO legacy_size_cast(FROM value)
{
    static_assert(std::is_unsigned_v<FROM> && std::is_unsigned_v<TO>,
                  "Only unsigned types can be cast here!");
    TO result = value;
    return result;
}

// ------------------------------------------------
// ------------- Net getter functions -------------
// ------------------------------------------------

inline std::string get_hostname(){
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    return hostname;
}

inline std::string get_nodename(){
    return cunqa::runtime_env::node_name();
}

inline std::string get_IP_address(const std::string& mode) 
{
    struct ifaddrs *interfaces, *ifa;
    
    if(mode == "hpc") 
        return "127.0.0.1"s;

    if (getifaddrs(&interfaces) == -1) {
        std::cerr << "Error getting the network interfaces\n";
        return std::string();
    }

    char ip[INET6_ADDRSTRLEN];
    for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || std::string(ifa->ifa_name) != "ib0") continue;
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            void *addr;
            addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(family, addr, ip, sizeof(ip));
            break;
        }
    }
    freeifaddrs(interfaces);
    return ip;
}

inline std::string get_global_IP_address() 
{
    struct ifaddrs *interfaces, *ifa;

    if (getifaddrs(&interfaces) == -1) {
        std::cerr << "Error getting the network interfaces\n";
        return std::string();
    }

    char ip[INET6_ADDRSTRLEN];
    for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || std::string(ifa->ifa_name) != "ib0") continue;
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            void *addr;
            addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(family, addr, ip, sizeof(ip));
            break;
        }
    }
    freeifaddrs(interfaces);
    return ip;
}
 
inline std::string get_port(const bool comm = false) 
{
    const std::string id = cunqa::runtime_env::proc_id();
    const std::string ports_str = cunqa::runtime_env::port_range();

    if(!ports_str.empty() && !id.empty()) {
        size_t pos = ports_str.find('-');
        if (pos != std::string::npos) {
            int numeric_id = std::stoi(id);
            if (comm) {
                std::string base_port_str = ports_str.substr(pos + 1);
                int base_port = std::stoi(base_port_str);
                return std::to_string(base_port - numeric_id);
            } else {
                std::string base_port_str = ports_str.substr(0, pos);
                int base_port = std::stoi(base_port_str);
                return std::to_string(base_port + numeric_id);
            }

        }
        throw std::runtime_error("Not a valid expression format of the ports.");
    }
    throw std::runtime_error("The required environment variables are not set (expected SLURM_STEP_RESV_PORTS or CUNQA_PORT_RANGE).");
}
