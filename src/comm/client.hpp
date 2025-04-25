#pragma once

#include <iostream>
#include <fstream>
#include <string_view>
#include <memory>
#include "comm_strat_def.h"
#include "utils/constants.hpp"
#include "logger/logger.hpp"
#include "config/net_config.hpp"
#include "future_wrapper.hpp"
#include "logger/logger.hpp"
#include "utils/json.hpp"

template <typename T>
class FutureWrapper {
public: 
    FutureWrapper(T * client) : client{client} {};
    
    inline std::string get() { return client->recv(); };
    inline bool AsioFuture::valid() { return true; };
}

class Client {
public:

    Client();
    ~Client();

    inline void connect(const std::string& ip, const std::string& port);
    inline FutureWrapper<Client> send_circuit(const std::string& circuit);
    inline FutureWrapper<Client> send_parameters(const std::string& parameters);
    inline std::string recv_results();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
