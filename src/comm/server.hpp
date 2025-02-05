#pragma once

#include <iostream>
#include <memory>
#include "config/net_config.hpp"
#include "comm-strat_def.h"

using namespace config;

#if COMM_LIB == ASIO
    #include "comm-strats/asio/asio_server.hpp"
    using SelectedServer = AsioServer;
#elif COMM_LIB == ZMQ
    #include "comm-strats/zmq_comm.hpp"
    using SelectedServer = ZMQServer;
#elif COMM_LIB == CROW
    #include "comm-strats/crow_comm.hpp"
    using SelectedServer = CrowServer;
#else
    #error "A valid library should be defined (ASIO, ZMQ o CROW) in COMM_LIB."
#endif

class Server {
    std::unique_ptr<SelectedServer> comm-strat;
public:

    Server(const NetConfig& net_config) :
        comm-strat{std::make_unique<SelectedServer>(net_config)} 
    { }

    inline std::string recv_circuit() { return comm-strat->recv_data(); }
    
    inline void accept() { comm-strat->accept(); }

    inline void send_result(const std::string& result) { comm-strat->send_result(result); }

    inline void close() {comm-strat->close(); }
};

