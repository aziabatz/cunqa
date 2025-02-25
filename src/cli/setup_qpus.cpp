#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <chrono>
#include <mpi.h>
#include <variant>

#include "qpu.hpp"
#include "simulators/simulator.hpp"
#include "config/qpu_config.hpp"
#include "comm/server.hpp"
#include "comm/client.hpp"
#include "logger/logger.hpp"
#include "utils/constants.hpp"

using json = nlohmann::json;

json get_qpu_config(json& backend_json)
{
    json qpu_config_json;
    if (backend_json.contains("fakeqmio_path")) {
        std::string command("python "s + std::getenv("INSTALL_PATH") + "/cunqa/fakeqmio.py "s + backend_json.at("fakeqmio_path").get<std::string>() + " "s +  std::getenv("SLURM_JOB_ID"));
        std::system(("ml load qmio/hpc gcc/12.3.0 qmio-tools/0.2.0-python-3.9.9 qiskit/1.2.4-python-3.9.9 2> /dev/null\n"s + command).c_str());
        std::string fq_path = std::getenv("STORE") + "/.api_simulator/tmp_fakeqmio_backend_"s + std::getenv("SLURM_JOB_ID") + ".json"s;
        std::ifstream f(fq_path);
        qpu_config_json = json::parse(f);
    
    } else if (backend_json.contains("backend_path")) {
        std::ifstream f(backend_json.at("backend_path"));
        qpu_config_json = json::parse(f);
    } else {
        throw std::runtime_error(std::string("Format not correct. Must be {\"backend_path\" : \"path/to/backend/json\"} or {\"fakeqmio_path\" : \"path/to/qmio/calibration/json\"}"));
    }

    return qpu_config_json;
}

//TODO: Try to overload both functions below
template<SimType sim_type>
void turn_on_qpu(int& mpi_rank, json& qpu_config_json, std::string& info_path)
{
    try{
        SPDLOG_LOGGER_DEBUG(logger, "Turning on QPUs.");
        config::QPUConfig<sim_type> qpu_config{qpu_config_json, info_path};
        QPU<sim_type> qpu(mpi_rank, qpu_config);
        SPDLOG_LOGGER_DEBUG(logger, "QPUs succesfully configured.");
        qpu.turn_ON();

    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning on QPUs because: \n\t{}", e.what());
    }
}




int main(int argc, char *argv[])
{
    SPDLOG_LOGGER_DEBUG(logger,"Setup QPUs arguments: argc={} argv= {} {} {} {}", argc, argv[1], argv[2], argv[3], argv[4]);
    std::string info_path(argv[1]);
    std::string communications(argv[2]);
    std::string simulator(argv[3]);
    std::string backend;
    json backend_json;
    int no_rank = -1;
    
    json qpu_config_json = {};
    try {
        if (argc == 5) {
            backend = std::string(argv[4]);
            std::cout << backend << "\n";
            backend_json = json::parse(backend);
            qpu_config_json = get_qpu_config(backend_json);
            
        } else  if (argc < 4) {
            SPDLOG_LOGGER_ERROR(logger, "Not a QPU configuration was given.");
        }

        switch (comm_map[communications])
        {
        case no_comm:
            if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
                if (search->second == SimType::Aer) {
                    turn_on_qpu<SimType::Aer>(no_rank, qpu_config_json, info_path);
                    SPDLOG_LOGGER_DEBUG(logger, "QPU with Aer turned on.");

                } else if (search->second == SimType::Munich) {
                    turn_on_qpu<SimType::Munich>(no_rank, qpu_config_json, info_path);
                    SPDLOG_LOGGER_DEBUG(logger, "QPU with Munich turned on.");
                }
            }
            break;

        case class_comm:
            SPDLOG_LOGGER_DEBUG(logger, "Ready to raise QPUs with classical communications.");
            // MPI INIT BLOCK //
            MPI_Init(&argc, &argv);

            int world_size;
            MPI_Comm_size(MPI_COMM_WORLD, &world_size);

            int world_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
            // MPI INIT BLOCK //
            
            if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
                if (search->second == SimType::Aer) {
                    turn_on_qpu<SimType::Aer>(world_rank, qpu_config_json, info_path);
                    SPDLOG_LOGGER_DEBUG(logger, "QPU with Aer turned on.");

                } else if (search->second == SimType::Munich) {
                    turn_on_qpu<SimType::Munich>(world_rank, qpu_config_json, info_path);
                    SPDLOG_LOGGER_DEBUG(logger, "QPU with Munich turned on.");
                }
            }            

            MPI_Finalize();
            break;

        case quantum_comm:
            SPDLOG_LOGGER_ERROR(logger, "Quantum communications are not implemented yet");
            return 0;
            break;

        } // End of switch
        
    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning ON the QPUs because: \n\t{}", e.what());
    }

    
}