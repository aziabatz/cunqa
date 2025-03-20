#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <chrono>
#include <optional>
#include <mpi.h>
#include "zmq.hpp"
#include <variant>

#include "qpu.hpp"
#include "simulators/simulator.hpp"
#include "config/qpu_config.hpp"
#include "config/net_config.hpp"
#include "comm/server.hpp"
#include "comm/client.hpp"
#include "comm/qpu_comm.hpp"
#include "logger/logger.hpp"
#include "utils/constants.hpp"

using json = nlohmann::json;

json get_qpu_config(json& backend_json)
{
    json qpu_config_json;
    if (backend_json.contains("fakeqmio_path")) {
        SPDLOG_LOGGER_DEBUG(logger, "PATH to Qmio calibrations was provided.");
        std::string command("python "s + std::getenv("INSTALL_PATH") + "/cunqa/fakeqmio.py "s + backend_json.at("fakeqmio_path").get<std::string>() + " "s +  std::getenv("SLURM_JOB_ID"));
        std::system(("ml load qmio/hpc gcc/12.3.0 qmio-tools/0.2.0-python-3.9.9 qiskit/1.2.4-python-3.9.9 2> /dev/null\n"s + command).c_str());
        std::string fq_path = std::getenv("STORE") + "/.api_simulator/tmp_fakeqmio_backend_"s + std::getenv("SLURM_JOB_ID") + ".json"s;
        SPDLOG_LOGGER_DEBUG(logger, "FakeQmio temporal configuration file created.");
        std::ifstream f(fq_path);
        SPDLOG_LOGGER_DEBUG(logger, "FakeQmio temporal configuration file read.");
        qpu_config_json = json::parse(f);
        SPDLOG_LOGGER_DEBUG(logger, "Json with FakeQmio configuration parsed.");
    
    } else if (backend_json.contains("backend_path")) {
        SPDLOG_LOGGER_DEBUG(logger, "PATH to a specific backend was provided.");
        std::ifstream f(backend_json.at("backend_path"));
        SPDLOG_LOGGER_DEBUG(logger, "Backend was read.");
        qpu_config_json = json::parse(f);
        SPDLOG_LOGGER_DEBUG(logger, "Backend was parsed.");
    } else {
        throw std::runtime_error(std::string("Format not correct. Must be {\"backend_path\" : \"path/to/backend/json\"} or {\"fakeqmio_path\" : \"path/to/qmio/calibration/json\"}"));
    }

    return qpu_config_json;
}

template<SimType sim_type>
void turn_on_qpu(json& qpu_config_json, std::string& info_path, std::string& communications)
{
    try{
        SPDLOG_LOGGER_DEBUG(logger, "Turning on QPUs.");
        config::QPUConfig<sim_type> qpu_config{qpu_config_json, info_path};
        QPU<sim_type> qpu(qpu_config);
        SPDLOG_LOGGER_DEBUG(logger, "QPUs succesfully configured.");
        qpu.turn_ON();

    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning on QPUs because: \n\t{}", e.what());
    }
}

#if defined(QPU_ZMQ)
template<SimType sim_type>
void turn_on_qpu(json& qpu_config_json, std::string& info_path, std::string& communications, ZMQSockets zmq_sockets)
{
    try{
        SPDLOG_LOGGER_DEBUG(logger, "Turning on QPUs.");
        config::QPUConfig<sim_type> qpu_config{qpu_config_json, info_path};
            QPU<sim_type> qpu(qpu_config, std::move(zmq_sockets));
        SPDLOG_LOGGER_DEBUG(logger, "QPUs succesfully configured.");
        qpu.turn_ON();

    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning on QPUs because: \n\t{}", e.what());
    }
}

std::string get_zmq_endpoint(const std::string_view& net = INFINIBAND) 
{
    auto id = std::getenv("SLURM_LOCALID");
    std::string ports = std::getenv("SLURM_STEP_RESV_PORTS");
    std::string port;
    SPDLOG_LOGGER_DEBUG(logger, "SLURM_STEP_RESV_PORTS: {}", ports);

    if(ports != "" && id != "")
    {
        size_t pos = ports.find('-');
        if (pos != std::string::npos) 
        {
            std::string base_port_str = ports.substr(pos + 1);
            int base_port = std::stoi(base_port_str);
            
            port = std::to_string(base_port - std::stoi(id));
            
        } else 
        {
            std::cerr << "Not a valid expression format of the ports.\n";
            return "-1"s;
        }
    } else 
    {
        std::cerr << "The required environment variables are not set (not in a SLURM job).\n";
        return "-1"s;
    }

    SPDLOG_LOGGER_DEBUG(logger, "Por selected: {}", port);

    std::unordered_map<std::string, std::string> IPs = get_IP_addresses();

    SPDLOG_LOGGER_DEBUG(logger, "IP addresses picked.");

    std::string endpoint = "tcp://" + IPs.at(std::string(net)) + ":" + port;

    SPDLOG_LOGGER_DEBUG(logger, "Endpoint created: {}.", endpoint);

    return endpoint;
}
#endif




int main(int argc, char *argv[])
{
    SPDLOG_LOGGER_DEBUG(logger,"Setup QPUs arguments: argc={} argv= {} {} {} {}", argc, argv[1], argv[2], argv[3], argv[4]);
    std::string info_path(argv[1]);
    std::string communications(argv[2]);
    std::string simulator(argv[3]);
    std::string backend_str;
    json backend_json;
    ZMQSockets zmq_sockets;
    
    json qpu_config_json = {};
    try {
        if (argc == 5) {
            backend_str = std::string(argv[4]);
            SPDLOG_LOGGER_DEBUG(logger, "A PATH to a backend was provided: {}", backend_str);
            backend_json = json::parse(backend_str);
            qpu_config_json = get_qpu_config(backend_json);
            SPDLOG_LOGGER_DEBUG(logger, "Json with QPU configuration created");
            
        } else  if (argc < 4) {
            SPDLOG_LOGGER_ERROR(logger, "Not a QPU configuration was given.");
        }

        switch (comm_map[communications])
        {
        case no_comm:
        {
            SPDLOG_LOGGER_DEBUG(logger, "Ready to raise QPUs without communications.");
            if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
                if (search->second == SimType::Aer) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with AerSimulator.");
                    turn_on_qpu<SimType::Aer>(qpu_config_json, info_path, communications);

                } else if (search->second == SimType::Munich) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with MunichSimulator.");
                    turn_on_qpu<SimType::Munich>(qpu_config_json, info_path, communications);

                } else if (search->second == SimType::Munich) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with CunqaSimulator.");
                    turn_on_qpu<SimType::Cunqa>(qpu_config_json, info_path, communications);
                }
            }
            break;
        }

        case class_comm:
        {
            SPDLOG_LOGGER_DEBUG(logger, "Ready to raise QPUs with classical communications.");
            #if defined(QPU_MPI)
            // MPI INIT BLOCK //
            MPI_Init(&argc, &argv);

            int mpi_size;
            MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

            int mpi_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
            // MPI INIT BLOCK //
            
            if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
                if (search->second == SimType::Aer) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with Aer and MPI.");
                    turn_on_qpu<SimType::Aer>(qpu_config_json, info_path, communications);
                    
                } else if (search->second == SimType::Munich) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with Munich and MPI.");
                    turn_on_qpu<SimType::Munich>(qpu_config_json, info_path, communications);

                } else if (search->second == SimType::Cunqa) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with CunqaSimulator and MPI.");
                    turn_on_qpu<SimType::Cunqa>(qpu_config_json, info_path, communications);
                }
            }            

            MPI_Finalize();

            #elif defined(QPU_ZMQ)
            //Client part
            zmq::context_t qpu_client_context_;
            zmq::socket_t qpu_client_socket_(qpu_client_context_, zmq::socket_type::client);
            SPDLOG_LOGGER_DEBUG(logger, "QPU client socket instanciated.");

            zmq_sockets.client = std::move(qpu_client_socket_);

            SPDLOG_LOGGER_DEBUG(logger, "zmq_sockets.client assigned.");

            //Server part
            zmq::context_t qpu_server_context_;
            zmq::socket_t qpu_server_socket_(qpu_server_context_, zmq::socket_type::server);

            SPDLOG_LOGGER_DEBUG(logger, "QPU server socket instanciated.");

            if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
                if (search->second == SimType::Aer) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with Aer and ZMQ.");

                    std::string zmq_endpoint = get_zmq_endpoint();
                    zmq_sockets.zmq_endpoint = zmq_endpoint;
                    qpu_server_socket_.bind(zmq_endpoint);
                    zmq_sockets.server = std::move(qpu_server_socket_);

                    SPDLOG_LOGGER_DEBUG(logger, "ZMQ server for QPU communications configured on endpoint {}.", zmq_endpoint);

                    turn_on_qpu<SimType::Aer>(qpu_config_json, info_path, communications, std::move(zmq_sockets));

                    
                } else if (search->second == SimType::Munich) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with Munich and ZMQ.");

                    std::string zmq_endpoint = get_zmq_endpoint();
                    zmq_sockets.zmq_endpoint = zmq_endpoint;
                    qpu_server_socket_.bind(zmq_endpoint);
                    zmq_sockets.server = std::move(qpu_server_socket_);

                    SPDLOG_LOGGER_DEBUG(logger, "ZMQ server for QPU communications configured on endpoint {}.", zmq_endpoint);

                    turn_on_qpu<SimType::Munich>(qpu_config_json, info_path, communications, std::move(zmq_sockets));

                } else if (search->second == SimType::Cunqa) {
                    SPDLOG_LOGGER_DEBUG(logger, "Turning on QPU with CunqaSimulator and ZMQ.");

                    std::string zmq_endpoint = get_zmq_endpoint();
                    SPDLOG_LOGGER_DEBUG(logger, "Generated ZMQ endpoint.");
                    zmq_sockets.zmq_endpoint = zmq_endpoint;
                    qpu_server_socket_.bind(zmq_endpoint);
                    SPDLOG_LOGGER_DEBUG(logger, "Server bound to ZMQ endpoint.");
                    zmq_sockets.server = std::move(qpu_server_socket_);

                    SPDLOG_LOGGER_DEBUG(logger, "ZMQ server for QPU communications configured on endpoint {}.", zmq_endpoint);

                    turn_on_qpu<SimType::Cunqa>(qpu_config_json, info_path, communications, std::move(zmq_sockets));
                }
            }
            
            #else
            SPDLOG_LOGGER_ERROR(logger, "No QPU-communication MACRO defined.");
            #endif

            break;
        }
        case quantum_comm:
            SPDLOG_LOGGER_ERROR(logger, "Quantum communications are not implemented yet");
            break;

        } // End of switch
        
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning ON the QPUs because: \n\t{}", e.what());
    }

    return 0;
    
}