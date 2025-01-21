#include <thread>
#include <queue>
#include <atomic>
#include <iostream>
#include <fstream>
#include <string>

#include <cstdlib>   // For rand() and srand()
#include <ctime> 

#include "comm/server.hpp"
#include "backend.hpp"
#include "simulators/simulator.hpp"
#include "config/qpu_config.hpp"
#include "utils/custom_json.hpp"
#include "utils/logger.hpp"

using json = nlohmann::json;
using namespace std::string_literals;
using namespace config;

template <SimType sim_type = SimType::Aer> 
class QPU {
public:
    config::QPUConfig<sim_type> qpu_config;
    Backend<sim_type> backend; 
    bool is_noise = false; 
    std::string backend_path; 

    QPU(config::QPUConfig<sim_type> qpu_config);

    QPU(config::QPUConfig<sim_type> qpu_config, const std::string& backend_path);

    void turn_ON();
    inline void turn_OFF();

private:
    std::unique_ptr<Server> server;
    std::queue<std::string> message_queue_;
    std::condition_variable queue_condition_;
    std::mutex queue_mutex_;

    void _compute_result();
    void _recv_data();
};


template <SimType sim_type>
QPU<sim_type>::QPU(config::QPUConfig<sim_type> qpu_config) : 
    qpu_config{qpu_config}
{
    CustomJson c_json{};
    json config_json(qpu_config);

    c_json.write(config_json, qpu_config.filepath);
} 

template <SimType sim_type>
QPU<sim_type>::QPU(config::QPUConfig<sim_type> qpu_config, const std::string& backend_path) : 
    qpu_config{qpu_config}, backend_path{backend_path}, backend{backend_path}
{
    CustomJson c_json{};
    json config_json(qpu_config);

    c_json.write(config_json, qpu_config.filepath);
} 

/* template <SimType sim_type> //Anterior constructor
QPU<sim_type>::QPU(config::QPUConfig<sim_type> qpu_config)
{
    if 
    CustomJson c_json{};
    json config_json(qpu_config);
    c_json.write(config_json, qpu_config.filepath);
} */



template <SimType sim_type>
void QPU<sim_type>::turn_ON() 
{
    server = std::make_unique<Server>(qpu_config.net_config);

    std::thread listen([this](){this->_recv_data();});
    std::thread compute([this](){this->_compute_result();});
    
    listen.join();
    compute.join();
}

template <SimType sim_type>
void QPU<sim_type>::_compute_result()
{    
    while (true) 
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_condition_.wait(lock, [this] { return !message_queue_.empty(); });

        while (!message_queue_.empty()) 
        {
            std::string message = message_queue_.front();
            message_queue_.pop();
            lock.unlock();

            json kernel = json::parse(message);

            //TODO: CAPTURAR AQUÍ CUALQUIER ERROR DEL SIMULADOR
            json response = backend.run(kernel, config::RunConfig(kernel.at("config")));

            try {
                server->send_result(to_string(response));
            } catch(const std::exception& e){
                SPDLOG_LOGGER_INFO(loggie::logger, "There has happened an error sending the result, the server keeps on iterating.");
                SPDLOG_LOGGER_ERROR(loggie::logger, "Official message of the error: {}", (std::string)e.what());
            }
            
            lock.lock();
        }
    }
}

template <SimType sim_type>
void QPU<sim_type>::_recv_data() 
{
    // TODO: Should the QPU be stoppable??
    while (true) 
    {
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
            SPDLOG_LOGGER_INFO(loggie::logger, "There has happened an error receiving the circuit, the server keeps on iterating.");
            SPDLOG_LOGGER_ERROR(loggie::logger, "Official message of the error: {}", (std::string)e.what());
        }
    }
}