#pragma once

#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>

#include "comm/server.hpp"
#include "backends/backend.hpp"
#include "utils/json.hpp"
#include "logger.hpp"


using namespace std::string_literals;

namespace cunqa {

class QPU {
public:    
    std::unique_ptr<sim::Backend> backend;
    std::unique_ptr<comm::Server> server;

    QPU(std::unique_ptr<sim::Backend> backend, const std::string& mode, const std::string& family_id);
    void turn_ON();

private:
    std::queue<std::string> message_queue_;
    std::condition_variable queue_condition_;
    std::mutex queue_mutex_;
    std::string family_id_;

    void compute_result_();
    void recv_data_();
    
    friend void to_json(JSON& j, const QPU& obj) {
        JSON backend_json = obj.backend->to_json();
        LOGGER_DEBUG("backend hecho.");
        JSON server_json = *(obj.server);
        LOGGER_DEBUG("server hecho.");
        j = {
            {"backend", backend_json},
            {"net", server_json},
            {"family_id", obj.family_id_}
        };
    }
};
        
} // End of cunqa namespace