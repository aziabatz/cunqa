#pragma once

#include <nlohmann/json.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;

using json = nlohmann::json;

class IPConfig {
public:

    std::string hostname;
    std::vector<std::string> IPs;
    std::string port;

    IPConfig(const int& n_port)
        : hostname{ip::host_name()},
          IPs{_getIPAddresses()},
          port{std::to_string(n_port)} { }

    IPConfig(const std::string& hostname, const int& n_port)
        : hostname{hostname},
          IPs{_getIPAddresses()},
          port{std::to_string(n_port)} { }
    
    IPConfig(const std::string& hostname, const std::string& n_port)
        : hostname{hostname},
          IPs{_getIPAddresses()},
          port{n_port} { }
    

    IPConfig& operator=(const IPConfig& other) {
        
        // Check for self-assignment
        if (this != &other) {
            hostname = other.hostname;
            IPs = other.IPs;
            port = other.port;
        }
        return *this;
    }

    json to_json(const int& qpu_id){
        json config_json = {
            {std::to_string(qpu_id), {
                { {"hostname", hostname}, {"port", port} },
            }}
        };
        return config_json;
    }

private:

    std::vector<std::string> _getIPAddresses() {
        io_context io_context;
        std::vector<std::string> ips;

        // Obtiene el nombre del host
        std::string host = ip::host_name();

        // Resuelve el nombre del host a direcciones
        ip::tcp::resolver resolver(io_context);
        ip::tcp::resolver::results_type endpoints = resolver.resolve(host, "");

        for (const auto& endpoint : endpoints) {
            ips.push_back(endpoint.endpoint().address().to_string());
        }

        return ips;
    }
};

std::ostream& operator<<(std::ostream& os, const IPConfig& config) {
    os << "\nIPs: ";
    for (auto ip: config.IPs){
        os << ip << " ";
    }
    os << "\nPuerto: " << config.port
       << "\nHostname: " << config.hostname << "\n\n";
    return os;
}