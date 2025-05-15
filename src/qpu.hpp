#pragma once

#include <string>
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

    QPU(std::unique_ptr<sim::Backend> backend, const std::string& mode, const std::string& family);
    void turn_ON();

private:
    std::queue<std::string> message_queue_;
    std::condition_variable queue_condition_;
    std::mutex queue_mutex_;
    std::string family_;

    void compute_result_();
    void recv_data_();
    
    friend void to_json(JSON& j, const QPU& obj) {
        JSON backend_json = obj.backend->to_json();
        JSON server_json = *(obj.server);
        std::string communications_endpoint = obj.backend->get_endpoint();
        LOGGER_DEBUG("QPU communications endpoint: {}", communications_endpoint);
        j = {
            {"backend", backend_json},
            {"net", server_json},
            {"family", obj.family_},
            {"slurm_job_id", std::getenv("SLURM_JOB_ID")},
            {"communications_endpoint", communications_endpoint}
        };
    }
};
        
} // End of cunqa namespace