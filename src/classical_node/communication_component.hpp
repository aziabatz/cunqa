#pragma once

#include <string>
#include "zmq.hpp"
#include <unordered_map>
#include <queue>
#include <chrono>
#include <thread>

#include "simulators/simulator.hpp"
#include "logger/logger.hpp"

template <SimType sim_type>
class CommunicationComponent
{
public:
    std::string comm_type;
    int mpi_size = -1;
    int mpi_rank = -1;
    std::optional<zmq::context_t> zmq_context;
    std::optional<std::unordered_map<std::string, zmq::socket_t>> zmq_comm_clients;
    std::optional<zmq::socket_t> zmq_comm_server;
    std::optional<std::string> zmq_endpoint;
    std::unordered_map<std::string, std::queue<int>> message_queue;

    CommunicationComponent(std::string& comm_type) : comm_type(comm_type)
    {
        if (comm_type == "mpi") {
            MPI_Init(NULL, NULL);
            MPI_Comm_size(MPI_COMM_WORLD, &(this->mpi_size));
            MPI_Comm_rank(MPI_COMM_WORLD, &(this->mpi_rank));
    
            SPDLOG_LOGGER_DEBUG(logger, "MPI communication of Communication Component configured.");
    
        } else if (comm_type == "zmq"){
            //Context
            zmq::context_t aux_context;
            zmq_context = std::move(aux_context);
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ context instanciated.");
    
            //Endpoint part
            std::string aux_endpoint = get_my_endpoint();
            zmq_endpoint = std::move(aux_endpoint); 
            SPDLOG_LOGGER_DEBUG(logger, "Endpoint created.");
    
            //Server part
            zmq::socket_t qpu_server_socket_(zmq_context.value(), zmq::socket_type::router);
            qpu_server_socket_.bind(zmq_endpoint.value());
            SPDLOG_LOGGER_DEBUG(logger, "Server bound to endpoint.");
            zmq_comm_server = std::move(qpu_server_socket_);
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ server socket instanciated.");
    
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ communication of Communication Component configured."); 

        } else if (comm_type == "no_comm") {
            SPDLOG_LOGGER_DEBUG(logger, "Communication component instanciated without communication endpoints.");

        }
    }

    inline void _send(int& measurement, std::string& destination);
    inline int _recv(std::string& origin);
    inline int is_sender_or_receiver(std::array<std::string, 2>& endpoints);

private:
};


template <SimType sim_type>
inline void CommunicationComponent<sim_type>::_send(int& measurement, std::string& destination) 
{
    if (this->comm_type == "mpi") {
        int destination_int = std::atoi(destination.c_str());
        MPI_Send(&measurement, 1, MPI_INT, destination_int, 1, MPI_COMM_WORLD);
    } else if (this->comm_type == "zmq") {
        //Client part
        if (!this->zmq_comm_clients.has_value()) {
            this->zmq_comm_clients.emplace();
            std::vector<std::string> others_endpoints = get_others_endpoints(this->zmq_endpoint.value());
            if (others_endpoints.empty()) {
                throw std::runtime_error("Impossible to get others endpoints.");
            }

            for (auto& endpoint : others_endpoints) {
                zmq::socket_t tmp_client_socket(this->zmq_context.value(), zmq::socket_type::dealer);
                tmp_client_socket.setsockopt(ZMQ_IDENTITY, this->zmq_endpoint.value().c_str(), this->zmq_endpoint.value().size());
                this->zmq_comm_clients.value()[endpoint] = std::move(tmp_client_socket);
            }
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ client sockets instanciated.");
        }
        
        this->zmq_comm_clients.value()[destination].connect(destination);
        zmq::message_t message(sizeof(int));
        std::memcpy(message.data(), &measurement, sizeof(int));
        this->zmq_comm_clients.value()[destination].send(message);
        SPDLOG_LOGGER_DEBUG(logger, "Measurement sent to: {}", destination);
    } else {
        throw std::runtime_error("Invalid communication type for string destination");
    }
}

template <SimType sim_type>
inline int CommunicationComponent<sim_type>::_recv(std::string& origin) 
{
    int measurement;
    if (this->comm_type == "mpi") {
        int origin_int = std::atoi(origin.c_str());
        MPI_Recv(&measurement, 1, MPI_INT, origin_int, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        return measurement;
    } else if (this->comm_type == "zmq") {
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

                this->zmq_comm_server.value().recv(client_id, zmq::recv_flags::none);
                this->zmq_comm_server.value().recv(message, zmq::recv_flags::none);
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
}

// 0->Sender, 1->Receiver
template <SimType sim_type>
inline int CommunicationComponent<sim_type>::is_sender_or_receiver(std::array<std::string, 2>& endpoints)
{
    int first_endpoint;
    int second_endpoint;
    switch(CUNQA::qpu_comm_map[this->comm_type]){
        case CUNQA::mpi:
            first_endpoint = std::atoi(endpoints[0].c_str());
            second_endpoint = std::atoi(endpoints[1].c_str());

            if (this->mpi_rank == first_endpoint) {
                return 0;
            } else if (this->mpi_rank == second_endpoint) {
                return 1;
            } else {
                return -1;
            }
            break;
        case CUNQA::zmq:
            if (this->zmq_endpoint.value() == endpoints[0]) {
                return 0;
            } else if (this->zmq_endpoint.value() == endpoints[1]) {
                return 1;
            } else {
                return -1;
            }
            break;

        default: 
            SPDLOG_LOGGER_ERROR(logger, "Error. This QPU is not sender nor receiver."); 
            throw std::runtime_error("This QPU is not sender nor receiver.");
            break;
        }

    return -1;

}