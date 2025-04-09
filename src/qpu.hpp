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
#include "custom_json.hpp"
#include "logger/logger.hpp"
#include "utils/parametric_circuit.hpp"
#include "utils/qpu_utils.hpp"
#include "classical_node/classical_node.hpp"



using json = nlohmann::json;
using namespace std::string_literals;
using namespace config;


template <SimType sim_type = SimType::Aer> 
class QPU {
public:

    config::QPUConfig<sim_type> qpu_config;
    //Backend<sim_type> backend;
    std::unique_ptr<QPUClassicalNode<sim_type>> classical_node;
    

    QPU(config::QPUConfig<sim_type> qpu_config, int& argc, char *argv[], std::string& comm_type = "None");

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
    json kernel = {};

    void _compute_result();
    void _recv_data();
    inline void _update_params(std::vector<double>& parameters);
    inline void _run_circuit(json& kernel, std::string& circ_type);
    inline void _update_circuit_params(std::vector<double>& parameters);
    inline void _update_qasm_params(std::vector<double>& parameters);
    
};



template <SimType sim_type>
QPU<sim_type>::QPU(config::QPUConfig<sim_type> qpu_config, int& argc, char *argv[], std::string& comm_type) : qpu_config{qpu_config}, comm_type(comm_type)
{
    CustomJson c_json{};
    json config_json(qpu_config);

    c_json.write(config_json, qpu_config.filepath);

    classical_node = std::make_unique<QPUClassicalNode<sim_type>>(comm_type, qpu_config, argc, argv);
    SPDLOG_LOGGER_DEBUG(logger, "QPU classical node configured.");
     
}

template <SimType sim_type>
void QPU<sim_type>::turn_ON() 
{
    SPDLOG_LOGGER_INFO(logger, "We set up the server.");
    server = std::make_unique<Server>(qpu_config.net_config);

    std::thread listen([this](){this->_recv_data();});
    std::thread compute([this](){this->_compute_result();});
    
    listen.join();
    compute.join();
}


template <SimType sim_type>
void QPU<sim_type>::_compute_result()
{    
    json kernel = {}; 
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

                SPDLOG_LOGGER_DEBUG(logger, "Message received.");
                json message_json = json::parse(message);
                SPDLOG_LOGGER_DEBUG(logger, "Message parsed.");

                if (message_json.contains("params")){ 
                    std::vector<double> parameters = message_json.at("params");
                    SPDLOG_LOGGER_DEBUG(logger, "Parameters received");
                    this->_update_params(parameters);
                } else {
                    SPDLOG_LOGGER_DEBUG(logger, "Circuit received");
                    this->exec_type = message_json.at("exec_type");
                    SPDLOG_LOGGER_DEBUG(logger, "Circuit type: {}.", exec_type);
                    message_json.erase("exec_type");
                    SPDLOG_LOGGER_DEBUG(logger, " \"type\" key deleted.");
                    this->kernel = message_json;
                }
                this->_run_circuit(this->kernel, this->exec_type);
                server->send_result(to_string(this->classical_node->result));
                this->classical_node->clean_result();

            } catch(const ServerException& e) {
                SPDLOG_LOGGER_ERROR(logger, "There has happened an error sending the result, probably the client has had an error.");
                SPDLOG_LOGGER_ERROR(logger, "Message of the error: {}", e.what());
            } catch(const std::exception& e) {
                SPDLOG_LOGGER_ERROR(logger, "There has happened an error sending the result, the server keeps on iterating.");
                SPDLOG_LOGGER_ERROR(logger, "Message of the error: {}", e.what());
                server->send_result("{\"ERROR\":\""s + std::string(e.what()) + "\"}"s);
            }
            lock.lock();
        }
    }
}



template <SimType sim_type>
void QPU<sim_type>::_run_circuit(json& kernel, std::string& exec_type)
{
    if(exec_type == "offloading") {
        std::chrono::steady_clock::time_point begin_run_time = std::chrono::steady_clock::now();
        SPDLOG_LOGGER_DEBUG(logger, "Circuit to be run offloading: {}", kernel.dump());
        this->classical_node->send_circ_to_execute(kernel);
        std::chrono::steady_clock::time_point end_run_time = std::chrono::steady_clock::now();
        SPDLOG_LOGGER_DEBUG(logger, "Offloading run time: {} [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end_run_time - begin_run_time).count());

    } else if (exec_type == "dynamic") {
        std::chrono::steady_clock::time_point begin_run_time = std::chrono::steady_clock::now();
        SPDLOG_LOGGER_DEBUG(logger, "Circuit to be run dynamically: {}", kernel.dump());
        this->classical_node->send_instructions_to_execute(kernel);
        std::chrono::steady_clock::time_point end_run_time = std::chrono::steady_clock::now();
        SPDLOG_LOGGER_DEBUG(logger, "Dynamic run time: {} [µs]", std::chrono::duration_cast<std::chrono::microseconds>(end_run_time - begin_run_time).count());
    } else {
        SPDLOG_LOGGER_ERROR(logger, "There was a problem when executing the circuit.");
        throw std::runtime_error("There was a problem when executing the circuit.");
    }
    
}


template<SimType sim_type>
void QPU<sim_type>::_update_params(std::vector<double>& parameters)
{
    if (this->classical_node->backend.backend_config.simulator == "AerSimulator" || this->classical_node->backend.backend_config.simulator == "CunqaSimulator"){
        SPDLOG_LOGGER_DEBUG(logger, "Params of AerSimulator or CunqaSimulator");
        if (!this->kernel.empty()){
            SPDLOG_LOGGER_DEBUG(logger, "Ready to update circuit parameters");
            this->_update_circuit_params(parameters);   
        } else {
            SPDLOG_LOGGER_ERROR(logger, "No parametric circuit was sent.");
            throw std::runtime_error("Parameters were sent before a parametric circuit.");
        }
    } else if (this->classical_node->backend.backend_config.simulator == "MunichSimulator") {
        SPDLOG_LOGGER_DEBUG(logger, "Params of MunichSimulator");
        if (!this->kernel.empty()){
            SPDLOG_LOGGER_DEBUG(logger, "Ready to update qasm parameters");;
            this->_update_qasm_params(parameters);
        } else {
            SPDLOG_LOGGER_ERROR(logger, "No parametric circuit was sent.");
            throw std::runtime_error("Parameters were sent before a parametric circuit.");
        }
    }
}

template <SimType sim_type>
inline void QPU<sim_type>::_update_circuit_params(std::vector<double>& parameters)
{
    if (!kernel.empty()){
        SPDLOG_LOGGER_DEBUG(logger, "Parameters were received");
        this->kernel = update_circuit_parameters(this->kernel, parameters);
        SPDLOG_LOGGER_DEBUG(logger, "Parametric circuit updated.");
    } else {
        SPDLOG_LOGGER_ERROR(logger, "No parametric circuit was sent.");
        throw std::runtime_error("Parameters were sent before a parametric circuit.");
    }
}

template <SimType sim_type>
inline void QPU<sim_type>::_update_qasm_params(std::vector<double>& parameters)
{
    if (!kernel.empty()){
        this->kernel = update_qasm_parameters(this->kernel, parameters);
        SPDLOG_LOGGER_DEBUG(logger, "Parametric circuit updated.");
    } else {
        SPDLOG_LOGGER_ERROR(logger, "No parametric circuit was sent.");
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
            SPDLOG_LOGGER_INFO(logger, "There has happened an error receiving the circuit, the server keeps on iterating.");
            SPDLOG_LOGGER_ERROR(logger, "Official message of the error: {}", e.what());
        }
    }
}