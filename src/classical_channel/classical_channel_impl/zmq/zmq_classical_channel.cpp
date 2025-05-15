
#include <string>
#include <queue>
#include "zmq.hpp"

#include "classical_channel.hpp"
#include "zmq_classical_channel_helpers.hpp"
#include "logger.hpp"

namespace cunqa {
namespace comm {

struct ClassicalChannel::Impl
{
    zmq::context_t zmq_context;
    std::unordered_map<std::string, zmq::socket_t> zmq_communication_clients;
    zmq::socket_t zmq_comm_server;
    std::string zmq_endpoint;
    std::unordered_map<std::string, std::queue<int>> message_queue;

    Impl()
    {
        //Context
        zmq::context_t aux_context;
        zmq_context = std::move(aux_context);
        SPDLOG_LOGGER_DEBUG(logger, "ZMQ context instanciated.");

        //Endpoint part
        zmq_endpoint = get_my_endpoint();
        SPDLOG_LOGGER_DEBUG(logger, "Endpoint created.");

        //Server part
        zmq::socket_t qpu_server_socket_(zmq_context, zmq::socket_type::router);
        qpu_server_socket_.bind(zmq_endpoint);
        SPDLOG_LOGGER_DEBUG(logger, "Server bound to endpoint.");
        zmq_comm_server = std::move(qpu_server_socket_);
        SPDLOG_LOGGER_DEBUG(logger, "ZMQ server socket instanciated.");

        SPDLOG_LOGGER_DEBUG(logger, "ZMQ communication of Communication Component configured."); 
    }
    ~Impl() = default;

    void send(int& measurement, std::string& target)
    {
        //Client part
        if (zmq_communication_clients.empty()) {
            std::vector<std::string> others_endpoints = get_others_endpoints(zmq_endpoint);
            if (others_endpoints.empty()) {
                throw std::runtime_error("Impossible to get others endpoints.");
            }

            for (auto& endpoint : others_endpoints) {
                zmq::socket_t tmp_client_socket(zmq_context, zmq::socket_type::dealer);
                tmp_client_socket.setsockopt(ZMQ_IDENTITY, zmq_endpoint.c_str(), zmq_endpoint.size());
                zmq_communication_clients[endpoint] = std::move(tmp_client_socket);
            }
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ client sockets instanciated.");
        }
        
        zmq_communication_clients[target].connect(target);
        zmq::message_t message(sizeof(int));
        std::memcpy(message.data(), &measurement, sizeof(int));
        zmq_communication_clients[target].send(message);
        SPDLOG_LOGGER_DEBUG(logger, "Measurement sent to: {}", target);
    }

    int recv(std::string& origin)
    {
        int measurement;
        if (!message_queue[origin].empty()) {
            SPDLOG_LOGGER_DEBUG(logger, "The message_queue already had a message from client {}.", origin);
            measurement = message_queue[origin].front();
            SPDLOG_LOGGER_DEBUG(logger, "Measurement extracted from the message_queue.");
            message_queue[origin].pop();
            SPDLOG_LOGGER_DEBUG(logger, "Measurement deleted from the message_queue.");
            return measurement;
        } else {
            while (true) {
                SPDLOG_LOGGER_DEBUG(logger, "Waiting for the message from {}.", origin);
                zmq::message_t client_id;
                zmq::message_t message;

                zmq_comm_server.recv(client_id, zmq::recv_flags::none);
                zmq_comm_server.recv(message, zmq::recv_flags::none);
                std::string client_id_str(static_cast<char*>(client_id.data()), client_id.size());
                std::memcpy(&measurement, message.data(), sizeof(int));
                
                if (client_id_str == origin) {
                    SPDLOG_LOGGER_DEBUG(logger, "The measurement came from the desired client.");
                    return measurement;
                } else {
                    SPDLOG_LOGGER_DEBUG(logger, "The measurement came from other client with id: {}", client_id_str);
                    this->message_queue[client_id_str].push(measurement);
                    SPDLOG_LOGGER_DEBUG(logger, "Message_queue updated.");
                }
            }
        }
    }

};


ClassicalChannel::ClassicalChannel() : pimpl_{std::make_unique<Impl>()}
{}

ClassicalChannel::~ClassicalChannel() = default;

void ClassicalChannel::send_measure(int& measurement, std::string& target)
{
    pimpl_->send(measurement, target);
}

int ClassicalChannel::recv_measure(std::string& origin)
{
    return pimpl_->recv(origin);
}


} // End of comm namespace
} // End of cunqa namespace
