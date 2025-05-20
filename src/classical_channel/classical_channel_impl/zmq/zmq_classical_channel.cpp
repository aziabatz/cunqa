
#include <string>
#include <queue>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include "zmq.hpp"

#include "classical_channel.hpp"
#include "zmq_classical_channel_helpers.hpp"
#include "logger.hpp"

namespace cunqa {
namespace comm {

struct ClassicalChannel::Impl
{
    std::unique_ptr<zmq::context_t> zmq_context;
    std::unordered_map<std::string, zmq::socket_t> zmq_connected_clients;
    zmq::socket_t zmq_comm_server;
    std::string zmq_endpoint;
    std::unordered_map<std::string, std::queue<int>> message_queue;

    Impl()
    {
        //Context
        zmq::context_t aux_context;
        zmq_context = std::make_unique<zmq::context_t>(std::move(aux_context));
        LOGGER_DEBUG("ZMQ context instanciated.");

        //Endpoint part
        zmq_endpoint = get_my_endpoint();
        LOGGER_DEBUG("Endpoint created.");

        //Server part
        zmq::socket_t qpu_server_socket_(*zmq_context, zmq::socket_type::router);
        qpu_server_socket_.bind(zmq_endpoint);
        LOGGER_DEBUG("Server bound to endpoint.");
        zmq_comm_server = std::move(qpu_server_socket_);
        LOGGER_DEBUG("ZMQ server socket instanciated.");

        LOGGER_DEBUG("ZMQ communication of Communication Component configured."); 
    }
    ~Impl() = default;

    void send(int& measurement, std::string& target)
    {
        // Client part
        if (zmq_connected_clients.find(target) == zmq_connected_clients.end()) {
            LOGGER_ERROR("No connections was established with endpoint {}.", target);
            throw std::runtime_error("Error with endpoint connection.");
        }
        zmq::message_t message(sizeof(int));
        std::memcpy(message.data(), &measurement, sizeof(int));
        zmq_connected_clients[target].send(message);
        LOGGER_DEBUG("Measurement sent to: {}", target);
    }

    int recv(std::string& origin)
    {
        int measurement;
        if (!message_queue[origin].empty()) {
            LOGGER_DEBUG("The message_queue already had a message from client {}.", origin);
            measurement = message_queue[origin].front();
            LOGGER_DEBUG("Measurement extracted from the message_queue.");
            message_queue[origin].pop();
            LOGGER_DEBUG("Measurement deleted from the message_queue.");
            return measurement;
        } else {
            while (true) {
                LOGGER_DEBUG("Waiting for the message from {}.", origin);
                zmq::message_t client_id;
                zmq::message_t message;

                zmq_comm_server.recv(client_id, zmq::recv_flags::none);
                zmq_comm_server.recv(message, zmq::recv_flags::none);
                std::string client_id_str(static_cast<char*>(client_id.data()), client_id.size());
                std::memcpy(&measurement, message.data(), sizeof(int));
                
                if (client_id_str == origin) {
                    LOGGER_DEBUG("The measurement came from the desired client.");
                    return measurement;
                } else {
                    LOGGER_DEBUG("The measurement came from other client with id: {}", client_id_str);
                    this->message_queue[client_id_str].push(measurement);
                    LOGGER_DEBUG("Message_queue updated.");
                }
            }
        }
    }

    void set_connections(std::vector<std::string>& endpoints)
    {
        for (auto& endpoint : endpoints) {
            if (zmq_connected_clients.find(endpoint) == zmq_connected_clients.end()) {
                zmq::socket_t tmp_client_socket(*zmq_context, zmq::socket_type::dealer);
                tmp_client_socket.setsockopt(ZMQ_IDENTITY, zmq_endpoint.c_str(), zmq_endpoint.size());
                zmq_connected_clients[endpoint] = std::move(tmp_client_socket);
                zmq_connected_clients[endpoint].connect(endpoint);
            }
        }
        LOGGER_DEBUG("ZMQ client sockets instanciated.");
    }
};


ClassicalChannel::ClassicalChannel() : pimpl_{std::make_unique<Impl>()}, endpoint{pimpl_->zmq_endpoint}
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

void ClassicalChannel::set_classical_connections(std::vector<std::string>& endpoints)
{
    LOGGER_DEBUG("Setting connections on classical channel.");
    pimpl_->set_connections(endpoints);
}


} // End of comm namespace
} // End of cunqa namespace
