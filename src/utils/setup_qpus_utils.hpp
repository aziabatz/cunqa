#pragma once

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
#include "utils/constants.hpp"
#include "utils/json.hpp"
#include "logger/logger.hpp"


cunqa::JSON get_qpu_config(cunqa::JSON& backend_json)
{
    cunqa::JSON qpu_config_json;
    if (backend_json.contains("fakeqmio_path")) {
        LOGGER_DEBUG("PATH to Qmio calibrations was provided.");
        std::string command("python "s + std::getenv("INSTALL_PATH") + "/cunqa/fakeqmio.py "s + backend_json.at("fakeqmio_path").get<std::string>() + " "s +  std::getenv("SLURM_JOB_ID"));
        std::system(("ml load qmio/hpc gcc/12.3.0 qmio-tools/0.2.0-python-3.9.9 qiskit/1.2.4-python-3.9.9 2> /dev/null\n"s + command).c_str());
        std::string fq_path = std::getenv("STORE") + "/.cunqa/tmp_fakeqmio_backend_"s + std::getenv("SLURM_JOB_ID") + ".cunqa::JSON"s;
        LOGGER_DEBUG("FakeQmio temporal configuration file created.");
        std::ifstream f(fq_path);
        LOGGER_DEBUG("FakeQmio temporal configuration file read.");
        qpu_config_json = cunqa::JSON::parse(f);
        LOGGER_DEBUG("Json with FakeQmio configuration parsed.");
    
    } else if (backend_json.contains("backend_path")) {
        LOGGER_DEBUG("PATH to a specific backend was provided.");
        std::ifstream f(backend_json.at("backend_path"));
        LOGGER_DEBUG("Backend was read.");
        qpu_config_json = cunqa::JSON::parse(f);
        LOGGER_DEBUG("Backend was parsed.");
    } else {
        throw std::runtime_error(std::string("Format not correct. Must be {\"backend_path\" : \"path/to/backend/json\"} or {\"fakeqmio_path\" : \"path/to/qmio/calibration/json\"}"));
    }

    return qpu_config_json;
}

template<SimType sim_type>
void turn_on_qpu(std::string& mode, cunqa::JSON& qpu_config_json, std::string& info_path, std::string& communications)
{
    try{
        LOGGER_DEBUG("Turning on QPUs.");
        config::QPUConfig<sim_type> qpu_config{mode, qpu_config_json, info_path};
        if (communications != "no_comm") {
            #if defined(QPU_MPI)
                std::string comm_type = "mpi";
                QPU<sim_type> qpu(qpu_config, comm_type);
                LOGGER_DEBUG("QPUs with MPI succesfully configured.");
                qpu.turn_ON();
            #elif defined(QPU_ZMQ)
                std::string comm_type = "zmq";
                QPU<sim_type> qpu(qpu_config, comm_type);
                LOGGER_DEBUG("QPUs with ZMQ succesfully configured.");
                qpu.turn_ON();
            #endif
        } else {
            QPU<sim_type> qpu(qpu_config, communications);
            LOGGER_DEBUG("QPUs with no communications configured.");
            qpu.turn_ON();
        }

    } catch (const std::exception& e) {
        LOGGER_ERROR("Failed turning on QPUs because: \n\t{}", e.what());
    }
}


