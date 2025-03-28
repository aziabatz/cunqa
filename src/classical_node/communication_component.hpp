#pragma once

#include <string>
#include "zmq.hpp"

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
            zmq::socket_t qpu_client_socket_(zmq_context.value(), zmq::socket_type::client);
            zmq_comm_client = std::move(qpu_client_socket_);
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ client socket instanciated.");
    
            //Endpoint part
            std::string aux_endpoint = get_zmq_endpoint();
            zmq_endpoint = std::move(aux_endpoint); 
            SPDLOG_LOGGER_DEBUG(logger, "Endpoint created.");
    
            //Server part
            zmq::socket_t qpu_server_socket_(zmq_context.value(), zmq::socket_type::server);
            qpu_server_socket_.bind(zmq_endpoint.value());
            SPDLOG_LOGGER_DEBUG(logger, "Server bound to endpoint.");
            zmq_comm_server = std::move(qpu_server_socket_);
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ server socket instanciated.");
    
            SPDLOG_LOGGER_DEBUG(logger, "ZMQ communication of Communication Component configured."); 

        } else if (comm_type == "no_comm") {
            SPDLOG_LOGGER_DEBUG(logger, "Communication component instanciated without communication endpoints.");

        }
    }
};