#include <string>
#include <iostream>
#include <stdexcept> //añadido para std::runtime_error
#include "qpu.hpp"
#include "logger.hpp"

using namespace std::string_literals;

// namespace {
//      TODO: not having this hardcoded here is an improvement for other supercomputing centers
//     const auto store = getenv("STORE");
//     const std::string filepath = store + "/.cunqa/qpus.json"s;
// }

namespace {
    // 1. Se crea una función para encapsular la lógica
    std::string get_qpus_json_path() {
        const char* store_env = getenv("STORE");
        if (!store_env) {
            // fallback razonable: usar HOME o lanzar error controlado
            const char* home_env = getenv("HOME");
            if (!home_env) {
                throw std::runtime_error("STORE environment variable not set and HOME not available");
            }
            return std::string(home_env) + "/.cunqa/qpus.json";
        } else {
            return std::string(store_env) + "/.cunqa/qpus.json";
        }
    }

    // 2. Se declara 'filepath' en el ámbito del namespace,
    //    inicializándola con la función.
    //    Ahora 'filepath' es visible para el resto del fichero.
    const std::string filepath = get_qpus_json_path();
}

namespace cunqa {

QPU::QPU(std::unique_ptr<sim::Backend> backend, const std::string& mode, const std::string& name, const std::string& family) :
    backend{std::move(backend)},
    server{std::make_unique<comm::Server>(mode)},
    name_{name},
    family_{family}
{ }

void QPU::turn_ON() 
{
    std::thread listen([this](){this->recv_data_();});
    std::thread compute([this](){this->compute_result_();});

    JSON qpu_config = *this;
    write_on_file(qpu_config, filepath, family_);

    listen.join();
    compute.join();
}

void QPU::compute_result_()
{    
    QuantumTask quantum_task_; 
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
                
                quantum_task_.update_circuit(message);
                auto result = backend->execute(quantum_task_);
                server->send_result(result.dump());

            } catch(const comm::ServerException& e) {
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

void QPU::recv_data_() 
{   
    while (true) {
        try {
            auto message = server->recv_data();
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


} // End of cunqa namespace

