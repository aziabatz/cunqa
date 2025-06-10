
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
    zmq::context_t zmq_context;
    std::unordered_map<std::string, zmq::socket_t> zmq_sockets;
    zmq::socket_t zmq_comm_server;
    std::string zmq_endpoint;
    std::unordered_map<std::string, std::queue<std::string>> message_queue;

    Impl()
    {
        //Endpoint part
        zmq_endpoint = get_my_endpoint();

        //Server part
        zmq::socket_t qpu_server_socket_(zmq_context, zmq::socket_type::router);
        qpu_server_socket_.bind(zmq_endpoint);
        zmq_comm_server = std::move(qpu_server_socket_);

        LOGGER_DEBUG("ZMQ communication of Communication Component configured."); 
    }
    ~Impl() = default;


    void connect(const std::string& endpoint)
    {
        if (zmq_sockets.find(endpoint) == zmq_sockets.end()) {
            zmq::socket_t tmp_client_socket(zmq_context, zmq::socket_type::dealer);
            tmp_client_socket.setsockopt(ZMQ_IDENTITY, zmq_endpoint.c_str(), zmq_endpoint.size());
            zmq_sockets[endpoint] = std::move(tmp_client_socket);
            zmq_sockets[endpoint].(endpoint);
        }
    }

    void send(const std::string& data, const std::string& target) 
    {
        if (zmq_sockets.find(target) == zmq_sockets.end()) {
            LOGGER_ERROR("No connections were established with endpoint {}.", target);
            throw std::runtime_error("Error with endpoint connection.");
        }
        zmq::message_t message(data.begin(), data.end());
        zmq_sockets[target].send(message, zmq::send_flags::none);
    }
    
    std::string recv(const std::string& origin)
    {
        if (!message_queue[origin].empty()) {
            std::string stored_data = message_queue[origin].front();
            message_queue[origin].pop();
            return stored_data;
        } else {
            while (true) {
                zmq::message_t id;
                zmq::message_t message;

                zmq_comm_server.recv(client_id, zmq::recv_flags::none);
                zmq_comm_server.recv(message, zmq::recv_flags::none);
                std::string id_str(static_cast<char*>(id.data()), id.size());
                std::string data(static_cast<char*>(id.data()), id.size());
                
                if (id_str == origin) {
                    return data;
                } else {
                    this->message_queue[client_id_str].push(data);
                }
            }
        }
    }
};


ClassicalChannel::ClassicalChannel() : pimpl_{std::make_unique<Impl>()} { endpoint = pimpl_->zmq_endpoint; }
ClassicalChannel::~ClassicalChannel() = default;

void ClassicalChannel::publish() 
{
    const std::string store = getenv("STORE");
    const std::string filepath = store + "/.cunqa/communications.json"s;
    JSON communications_endpoint = 
    {
        {"communications_endpoint", pimpl_->endpoint}
    };
    write_on_file(communications_endpoint, filepath);
}


//--------------------------------------------------
// Functions to stablish the other devices connected
//--------------------------------------------------
void ClassicalChannel::connect(const std::string& endpoint) { pimpl_->connect(endpoint); }

void ClassicalChannel::connect(const std::vector<std::string>& endpoints) 
{
    for (const auto& endpoints: endpoint) {
        pimpl_->connect(endpoint);
    }
}

//------------------------------------------------------------------------------------
// Send and recv functions for arbitrary info (such as a whole circuit or an endpoint)
//------------------------------------------------------------------------------------
void ClassicalChannel::send_info(const std::string& data, const std::string& target) { pimpl_->send(data, target); }
std::string ClassicalChannel::recv_info(const std::string& origin) { return pimpl_->recv(origin); }

//-----------------------------------------
// Send and recv functions for measurements
//-----------------------------------------
void ClassicalChannel::send_measure(const int& measurement, const std::string& target) { pimpl_->send(std::to_string(data), target); }
int ClassicalChannel::recv_measure(const std::string& origin) { return std::stoi(pimpl_->recv(origin)); }


} // End of comm namespace
} // End of cunqa namespace
