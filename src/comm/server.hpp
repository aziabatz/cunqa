#pragma once

#include <iostream>
#include <memory>
#include "config/net_config.hpp"
#include "comm_strat_def.h"

using namespace config;

#if COMM_LIB == ASIO
    #include "comm-strats/asio/asio_server.hpp"
    using SelectedServer = AsioServer;
#elif COMM_LIB == ZMQ
    #include "comm-strats/zmq/zmq_server.hpp"
    using SelectedServer = ZmqServer;
#elif COMM_LIB == CROW
    #include "comm-strats/crow_comm.hpp"
    using SelectedServer = CrowServer;
#else
    #error "A valid library should be defined (ASIO, ZMQ o CROW) in COMM_LIB."
#endif

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

    Server(const NetConfig& net_config) :
        comm_strat{std::make_unique<SelectedServer>(net_config)} 
    { }

    inline std::string recv_circuit() { return comm_strat->recv_data(); }
    
    inline void accept() { comm_strat->accept(); }

    inline void send_result(const std::string& result) { 
        try {
            comm_strat->send_result(result);
        } catch (const std::exception& e) {
            throw ServerException(e.what());
        }
    }

    inline void close() {comm_strat->close(); }
};

