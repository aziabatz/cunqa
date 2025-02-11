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
#include "custom_json.hpp"
#include "logger/logger.hpp"
#include "utils/parametric_circuit.hpp"

using json = nlohmann::json;
using namespace std::string_literals;
using namespace config;

template <SimType sim_type = SimType::Aer> 
class QPU {
public:
    config::QPUConfig<sim_type> qpu_config;
    Backend<sim_type> backend;

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
    qpu_config{qpu_config}, backend{qpu_config.backend_config}
{
    CustomJson c_json{};
    json config_json(qpu_config);

    c_json.write(config_json, qpu_config.filepath);
}


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

                SPDLOG_LOGGER_DEBUG(logger, "Message received:");
                json message_json = json::parse(message);

                // This does not refer to the field `params` for a specific gate, but
                // for a separated field specifying the new set of parameters
                if (!message_json.contains("params")){ 
                    SPDLOG_LOGGER_DEBUG(logger, "A circuit was received {}", message); 
                    kernel = message_json;
                    json response = backend.run(kernel);
                    server->send_result(to_string(response));
                    
                } else {
                    std::vector<double> parameters = message_json.at("params");
                    if (kernel.empty()){
                        SPDLOG_LOGGER_ERROR(logger, "No parametric circuit was sent.");
                        throw std::runtime_error("Parameters were sent before a parametric circuit.");
                    } else {
                        SPDLOG_LOGGER_DEBUG(logger, "Parameters were received {}", message);
                        kernel = update_circuit_parameters(kernel, parameters);
                        SPDLOG_LOGGER_DEBUG(logger, "Parametric circuit upgraded.");
                        parameters.clear();
                        json response = backend.run(kernel);
                        server->send_result(to_string(response));
                    }
                } 
            } catch(const boost::system::system_error& e) {
                SPDLOG_LOGGER_ERROR(logger, "There has happened an error sending the result, probably the client has had an error.");
                SPDLOG_LOGGER_ERROR(logger, "Boost message of the error: {}", e.what());
            } catch(const std::exception& e) {
                SPDLOG_LOGGER_ERROR(logger, "There has happened an error sending the result, the server keeps on iterating.");
                SPDLOG_LOGGER_ERROR(logger, "Message of the error: {}", e.what());
                server->send_result("{\"ERROR\":"s + e.what() + "}"s);
                
            }
            lock.lock();
        }
        
    }
}

template <SimType sim_type>
void QPU<sim_type>::_recv_data() 
{
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
            SPDLOG_LOGGER_INFO(logger, "There has happened an error receiving the circuit, the server keeps on iterating.");
            SPDLOG_LOGGER_ERROR(logger, "Official message of the error: {}", e.what());
        }
    }
}