#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include "strategy_def.h"
#include "utils/constants.hpp"
#include "utils/logger.hpp"
#include "config/net_config.hpp"
#include "future_wrapper.hpp"

using json = nlohmann::json;
using namespace config;

#if COMM_LIB == ASIO
    #include "strategies/asio/asio_client.hpp"
    using SelectedClient = AsioClient;
    using Future = AsioFuture;
#elif COMM_LIB == ZMQ
    #include "strategy/zmq_comm.hpp"
    using SelectedClient = ZMQClient;
    using Future = std::future<std::string>;
#elif COMM_LIB == CROW
    #include "strategy/crow_comm.hpp"
    using SelectedClient = CrowClient;
    using Future = std::future<std::string>;
#else
    #error "A valid library should be defined (ASIO, ZMQ o CROW) in COMM_LIB."
#endif

class Client {
    std::unique_ptr<SelectedClient> strategy;
    json qpus_json;

public:

    Client(const std::optional<std::string> &filepath) :
        strategy{std::make_unique<SelectedClient>()} 
    { 
        std::string final_filepath;
        if (filepath.has_value())
            final_filepath = filepath.value();
        else
            final_filepath = std::getenv("STORE") + "/.api_simulator/qpu.json"s;
        std::ifstream file(final_filepath);  

        if (!file.is_open()) {
            std::cerr << "Cannot open the JSON file\n";
        }

        try {
            file >> qpus_json;
        } catch (const json::parse_error& e) {
            std::cerr << "Error parsing the QPU info into JSON: " << e.what() << "\n";
        }
    }

    void connect(const std::string& task_id, const std::string_view& net = INFINIBAND) {
        try {
            json server_ip_config_json = qpus_json.at(task_id).at("net");
            auto server_ip_config = server_ip_config_json.template get<NetConfig>();
            strategy->connect(server_ip_config);
        } catch (const json::out_of_range& e){
            SPDLOG_LOGGER_ERROR(loggie::logger, "No server has ID={}. Remember to set the servers with the command qraise.", task_id);
        }
    }

    inline FutureWrapper send_circuit(const std::string& circuit) { return FutureWrapper(strategy->submit(circuit)); }
};
