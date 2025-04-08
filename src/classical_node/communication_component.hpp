#pragma once

#include <string>
#include "zmq.hpp"
#include <unordered_map>
#include <queue>

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
    std::optional<zmq::socket_t> zmq_comm_client;
    std::optional<zmq::socket_t> zmq_comm_server;
    std::optional<std::string> zmq_endpoint;
    std::unordered_map<std::string, std::queue<int>> message_queue;

    CommunicationComponent(std::string& comm_type, int& argc, char *argv[]) : comm_type(comm_type)
    {
        if (comm_type == "mpi") {
            MPI_Init(&argc, &argv);
            MPI_Comm_size(MPI_COMM_WORLD, &(this->mpi_size));
            MPI_Comm_rank(MPI_COMM_WORLD, &(this->mpi_rank));
    
            SPDLOG_LOGGER_DEBUG(logger, "MPI communication of Communication Component configured.");
    
        } else if (comm_type == "zmq"){
            //Context
            zmq::context_t aux_context;
            zmq_context = std::move(aux_context);
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ context instanciated.");
    
            //Client part
            zmq::socket_t qpu_client_socket_(zmq_context.value(), zmq::socket_type::dealer);
            zmq_comm_client = std::move(qpu_client_socket_);
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ client socket instanciated.");
    
            //Endpoint part
            std::string aux_endpoint = get_zmq_endpoint();
            this->zmq_comm_client->setsockopt(ZMQ_IDENTITY, aux_endpoint.c_str(), aux_endpoint.size());
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
    inline bool is_sender_qpu(std::string& endpoint);
    inline bool is_receiver_qpu(std::string& endpoint);

private:
    inline bool _check_endpoint(std::string& endpoint);
};


template <SimType sim_type>
inline void CommunicationComponent<sim_type>::_send(int& measurement, std::string& destination) 
{
    if (this->comm_type == "mpi") {
        int destination_int = std::atoi(destination.c_str());
        MPI_Send(&measurement, 1, MPI_INT, destination_int, 1, MPI_COMM_WORLD);
    } else if (this->comm_type == "zmq") {
        this->zmq_comm_client.value().connect(destination);
        //this->_send_client_id();
        zmq::message_t message(sizeof(int));
        std::memcpy(message.data(), &measurement, sizeof(int));
        this->zmq_comm_client.value().send(message);
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

template <SimType sim_type>
inline bool CommunicationComponent<sim_type>::_check_endpoint(std::string& endpoint)
{
    if (this->comm_type == "mpi") {
        int endpoint_int = std::atoi(endpoint.c_str());
        return endpoint_int == this->mpi_rank; 
    } else if (this->comm_type == "zmq") {
        return endpoint == this->zmq_endpoint;
    } else {
        return false;
    }
}

template <SimType sim_type>
inline bool CommunicationComponent<sim_type>::is_sender_qpu(std::string& endpoint)
{
    return this->_check_endpoint(endpoint);
}

template <SimType sim_type>
inline bool CommunicationComponent<sim_type>::is_receiver_qpu(std::string& endpoint)
{
    return this->_check_endpoint(endpoint);
}

