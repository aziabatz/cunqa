#pragma once

#include <iostream>
#include <memory>
#include "config/net_config.hpp"
#include "comm_strat_def.h"

class ServerException : public std::exception {
    std::string message;
public:
    explicit ServerException(const std::string& msg) : message(msg) { }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class Server {
    std::unique_ptr<SelectedServer> comm_strat;
public:
    std::string mode;
    std::string hostname;
    std::string nodename;
    std::unordered_map<std::string, std::string> IPs;
    std::string port;

    Server();

    inline void accept();
    inline std::string recv_data();
    inline void send_results(const std::string& results);
    inline void close();
};

