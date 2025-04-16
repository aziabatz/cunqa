#pragma once

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
        std::string fq_path = std::getenv("STORE") + "/.cunqa/tmp_fakeqmio_backend_"s + std::getenv("SLURM_JOB_ID") + ".json"s;
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
void turn_on_qpu(std::string& mode, json& qpu_config_json, std::string& info_path, std::string& communications)
{
    try{
        SPDLOG_LOGGER_DEBUG(logger, "Turning on QPUs.");
        config::QPUConfig<sim_type> qpu_config{mode, qpu_config_json, info_path};
        if (communications != "no_comm") {
            #if defined(QPU_MPI)
                std::string comm_type = "mpi";
                QPU<sim_type> qpu(qpu_config, comm_type);
                SPDLOG_LOGGER_DEBUG(logger, "QPUs with MPI succesfully configured.");
                qpu.turn_ON();
            #elif defined(QPU_ZMQ)
                std::string comm_type = "zmq";
                QPU<sim_type> qpu(qpu_config, comm_type);
                SPDLOG_LOGGER_DEBUG(logger, "QPUs with ZMQ succesfully configured.");
                qpu.turn_ON();
            #endif
        } else {
            QPU<sim_type> qpu(qpu_config, communications);
            SPDLOG_LOGGER_DEBUG(logger, "QPUs with no communications configured.");
            qpu.turn_ON();
        }

    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Failed turning on QPUs because: \n\t{}", e.what());
    }
}


