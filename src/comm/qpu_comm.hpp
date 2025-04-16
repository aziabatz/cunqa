#pragma once

#include "zmq.hpp"

struct ZMQSockets
{
    zmq::socket_t client;
    zmq::socket_t server;
    std::string zmq_endpoint;
};
