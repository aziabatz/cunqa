#pragma once

#include <thread>
#include <queue>
#include <atomic>
#include <iostream>
#include <fstream>
#include <string>
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
using namespace config;

template <SimType sim_type = SimType::Aer> 
class QPU {
public:
    config::QPUConfig<sim_type> qpu_config;
    std::unique_ptr<QPUClassicalNode<sim_type>> classical_node;
    
    QPU(config::QPUConfig<sim_type> qpu_config, std::string& comm_type);

    void turn_ON();
    inline void turn_OFF();

private:
    std::unique_ptr<Server> server;
    std::queue<std::string> message_queue_;
    std::condition_variable queue_condition_;
    std::mutex queue_mutex_;
    std::string communications;
    std::string comm_type;
    std::string exec_type;
    cunqa::JSON kernel = {};

    void _compute_result();
    void _recv_data();
    inline void _update_params(std::vector<double>& parameters);
    inline void _run_circuit(cunqa::JSON& kernel, std::string& circ_type);
    inline void _update_circuit_params(std::vector<double>& parameters);
    inline void _update_qasm_params(std::vector<double>& parameters);
    
};



template <SimType sim_type>
QPU<sim_type>::QPU(config::QPUConfig<sim_type> qpu_config, std::string& comm_type) : qpu_config{qpu_config}, comm_type(comm_type)
{
    LOGGER_INFO("QPU instanciated.");
    cunqa::JSON config_json(qpu_config);

    classical_node = std::make_unique<QPUClassicalNode<sim_type>>(comm_type, qpu_config);
    LOGGER_DEBUG("QPU classical node configured.");

    config_json["comm_info"] = this->classical_node->endpoint_info;
    cunqa::write_on_file(config_json, qpu_config.filepath);
    LOGGER_INFO("Write finished.");
}

template <SimType sim_type>
void QPU<sim_type>::turn_ON() 
{
    LOGGER_INFO("We set up the server.");
    server = std::make_unique<Server>(qpu_config.net_config);

    std::thread listen([this](){this->_recv_data();});
    std::thread compute([this](){this->_compute_result();});
    
    listen.join();
    compute.join();
}


template <SimType sim_type>
void QPU<sim_type>::_compute_result()
{    
    cunqa::JSON kernel = {}; 
    while (true) 
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_condition_.wait(lock, [this] { return !message_queue_.empty(); });

        while (!message_queue_.empty()) 
        {
            try {
                std::string message = message_queue_.front();
                message_queue_.pop();
                lock.unlock();

                LOGGER_DEBUG("Message received.");
                cunqa::JSON message_json = cunqa::JSON::parse(message);
                LOGGER_DEBUG("Message parsed.");

                if (message_json.contains("params")){ 
                    std::vector<double> parameters = message_json.at("params");
                    LOGGER_DEBUG("Parameters received");
                    this->_update_params(parameters);
                } else {
                    LOGGER_DEBUG("Circuit received");
                    this->exec_type = message_json.at("exec_type");
                    LOGGER_DEBUG("Circuit type: {}.", exec_type);
                    message_json.erase("exec_type");
                    LOGGER_DEBUG(" \"type\" key deleted.");
                    this->kernel = message_json;
                }
                this->_run_circuit(this->kernel, this->exec_type);
                server->send_result(to_string(this->classical_node->result));
                this->classical_node->clean_result();

            } catch(const ServerException& e) {
                LOGGER_ERROR("There has happened an error sending the result, probably the client has had an error.");
                LOGGER_ERROR("Message of the error: {}", e.what());
            } catch(const std::exception& e) {
                LOGGER_ERROR("There has happened an error sending the result, the server keeps on iterating.");
                LOGGER_ERROR("Message of the error: {}", e.what());
                server->send_result("{\"ERROR\":\""s + std::string(e.what()) + "\"}"s);
            }
            lock.lock();
        }
    }
}



template <SimType sim_type>
void QPU<sim_type>::_run_circuit(cunqa::JSON& kernel, std::string& exec_type)
{
    if(exec_type == "offloading") {
        std::chrono::steady_clock::time_point begin_run_time = std::chrono::steady_clock::now();
        LOGGER_DEBUG("Circuit to be run offloading: {}", kernel.dump());
        this->classical_node->send_circ_to_execute(kernel);
        std::chrono::steady_clock::time_point end_run_time = std::chrono::steady_clock::now();
        LOGGER_DEBUG("Offloading run time: {} [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end_run_time - begin_run_time).count());

    } else if (exec_type == "dynamic") {
        std::chrono::steady_clock::time_point begin_run_time = std::chrono::steady_clock::now();
        LOGGER_DEBUG("Circuit to be run dynamically: {}", kernel.dump());
        this->classical_node->send_instructions_to_execute(kernel);
        std::chrono::steady_clock::time_point end_run_time = std::chrono::steady_clock::now();
        LOGGER_DEBUG("Dynamic run time: {} [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end_run_time - begin_run_time).count());
    } else {
        LOGGER_ERROR("There was a problem when executing the circuit.");
        throw std::runtime_error("There was a problem when executing the circuit.");
    }
    
}


template<SimType sim_type>
void QPU<sim_type>::_update_params(std::vector<double>& parameters)
{
    if (this->classical_node->backend.backend_config.simulator == "AerSimulator" || this->classical_node->backend.backend_config.simulator == "CunqaSimulator"){
        LOGGER_DEBUG("Params of AerSimulator or CunqaSimulator");
        if (!this->kernel.empty()){
            LOGGER_DEBUG("Ready to update circuit parameters");
            this->_update_circuit_params(parameters);   
        } else {
            LOGGER_ERROR("No parametric circuit was sent.");
            throw std::runtime_error("Parameters were sent before a parametric circuit.");
        }
    } else if (this->classical_node->backend.backend_config.simulator == "MunichSimulator") {
        LOGGER_DEBUG("Params of MunichSimulator");
        if (!this->kernel.empty()){
            LOGGER_DEBUG("Ready to update qasm parameters");;
            this->_update_qasm_params(parameters);
        } else {
            LOGGER_ERROR("No parametric circuit was sent.");
            throw std::runtime_error("Parameters were sent before a parametric circuit.");
        }
    }
}

template <SimType sim_type>
inline void QPU<sim_type>::_update_circuit_params(std::vector<double>& parameters)
{
    if (!kernel.empty()){
        LOGGER_DEBUG("Parameters were received");
        this->kernel = update_circuit_parameters(this->kernel, parameters);
        LOGGER_DEBUG("Parametric circuit updated.");
    } else {
        LOGGER_ERROR("No parametric circuit was sent.");
        throw std::runtime_error("Parameters were sent before a parametric circuit.");
    }
}

template <SimType sim_type>
inline void QPU<sim_type>::_update_qasm_params(std::vector<double>& parameters)
{
    if (!kernel.empty()){
        this->kernel = update_qasm_parameters(this->kernel, parameters);
        LOGGER_DEBUG("Parametric circuit updated.");
    } else {
        LOGGER_ERROR("No parametric circuit was sent.");
        throw std::runtime_error("Parameters were sent before a parametric circuit.");
    }
}


template <SimType sim_type>
void QPU<sim_type>::_recv_data() 
{   
    while (true) {
        try {
            auto message = server->recv_circuit();
                {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (message.compare("CLOSE"s) == 0) {
                    server->accept();
                    continue;
                }
                else
                    message_queue_.push(message);
            }
            queue_condition_.notify_one();
        } catch (const std::exception& e) {
            LOGGER_INFO("There has happened an error receiving the circuit, the server keeps on iterating.");
            LOGGER_ERROR("Official message of the error: {}", e.what());
        }
    }
}