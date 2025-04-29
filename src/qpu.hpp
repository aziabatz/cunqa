#pragma once

#include <thread>
#include <queue>
#include <atomic>
#include <iostream>
#include <fstream>

#include <optional>
#include "zmq.hpp"
#include <cstdlib>   // For rand() and srand()
#include <chrono> 

#include "comm/server.hpp"
#include "comm/qpu_comm.hpp"
#include "backend.hpp"
#include "simulators/simulator.hpp"
#include "config/qpu_config.hpp"
#include "utils/json.hpp"
#include "logger/logger.hpp"
#include "utils/parametric_circuit.hpp"
#include "utils/qpu_utils.hpp"
#include "classical_node/classical_node.hpp"

using namespace std::string_literals;

namespace cunqa {

class QPU {
public:    
    std::unique_ptr<Backend> backend;
    std::unique_ptr<Server> server;

    QPU(Backend backend, std::string& comm_type);
    void turn_ON();

private:
    std::queue<std::string> message_queue_;
    std::condition_variable queue_condition_;
    std::mutex queue_mutex_;
    QuantumTask quantum_task;

    void compute_result_();
    void recv_data_();
    
    friend void to_json(JSON &j, QPU obj) {
        //
    }

    friend void from_json(JSON j, QPU &obj) {
        //
    }
};
        
}