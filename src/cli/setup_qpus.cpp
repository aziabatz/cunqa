
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <chrono>
#include <optional>
#include <mpi.h>
#include <variant>
#include "zmq.hpp"

#include "utils/json.hpp"
#include "utils/setup_qpus_utils.hpp"


int main(int argc, char *argv[])
{
    LOGGER_DEBUG("Setup QPUs with argc={} arguments", argc);
    std::string info_path(argv[1]);
    std::string mode(argv[2]);
    std::string communications(argv[3]);
    std::string family_name(argv[4]);
    std::string simulator(argv[5]);
    std::string backend;
    cunqa::JSON backend_json;
    
    cunqa::JSON qpu_config_json = {};
    try {
        if (argc == 7) {
            backend = std::string(argv[6]);
            backend_json = cunqa::JSON::parse(backend);
            if (backend_json.contains("fakeqmio_path")) {
                std::string command("python "s + std::getenv("INSTALL_PATH") + "/cunqa/fakeqmio.py "s + backend_json.at("fakeqmio_path").get<std::string>() + " "s +  std::getenv("SLURM_JOB_ID"));
                std::system(("ml load qmio/hpc gcc/12.3.0 qmio-tools/0.2.0-python-3.9.9 qiskit/1.2.4-python-3.9.9 2> /dev/null\n"s + command).c_str());
                std::string fq_path = std::getenv("STORE") + "/.cunqa/tmp_fakeqmio_backend_"s + std::getenv("SLURM_JOB_ID") + ".json"s;
                std::ifstream f(fq_path);
                qpu_config_json = cunqa::JSON::parse(f);
            } else if (backend_json.contains("backend_path")) {
                std::ifstream f(backend_json.at("backend_path").get<std::string>());
                qpu_config_json = cunqa::JSON::parse(f);
            } else {
                throw std::runtime_error(std::string("Format not correct. Must be {\"backend_path\" : \"path/to/backend/json\"} or {\"fakeqmio_path\" : \"path/to/qmio/calibration/json\"}"));
            }
            
        } else  if (argc < 6) {
            LOGGER_ERROR("Not a QPU configuration was given.");
        }

        if (family_name == "default"){
            family_name = (std::string)std::getenv("SLURM_JOB_ID");
        }

        qpu_config_json["family_name"] = family_name;
        qpu_config_json["slurm_job_id"] = (std::string)std::getenv("SLURM_JOB_ID");

        if(auto search = SIM_NAMES.find(simulator); search != SIM_NAMES.end()) {
            if (search->second == SimType::Aer) {
                LOGGER_DEBUG("Turning on QPU with AerSimulator.");
                turn_on_qpu<SimType::Aer>(mode, qpu_config_json, info_path, communications);

            } else if (search->second == SimType::Munich) {
                LOGGER_DEBUG("Turning on QPU with MunichSimulator.");
                turn_on_qpu<SimType::Munich>(mode, qpu_config_json, info_path, communications);

            } else if (search->second == SimType::Cunqa) {
                LOGGER_DEBUG("Turning on QPU with CunqaSimulator.");
                turn_on_qpu<SimType::Cunqa>(mode, qpu_config_json, info_path, communications);
            }
        }

        
    } catch (const std::exception& e) {
        LOGGER_ERROR("Failed turning ON the QPUs because: \n\t{}", e.what());
    }

    return 0;
    
}