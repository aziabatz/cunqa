
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "qpu.hpp"
#include "backends/simple_backend.hpp"
#include "backends/simulators/AER/aer_simple_simulator.hpp"
#include "backends/simulators/Munich/munich_simple_simulator.hpp"
#include "backends/simulators/CUNQA/cunqa_simple_simulator.hpp"
#include "backends/simulators/CUNQA/cunqa_classical_comm_simulator.hpp"

#include "utils/json.hpp"
#include "utils/helpers/murmur_hash.hpp"
#include "logger.hpp"

using namespace std::string_literals;

using namespace cunqa;
using namespace cunqa::sim;

template<typename Simulator, typename Config, typename BackendType>
void turn_ON_QPU(const JSON& backend_json, const std::string& mode, const std::string& family)
{
    std::unique_ptr<Simulator> simulator = std::make_unique<Simulator>();
    LOGGER_DEBUG("QPU simulator selected.");
    JSON config_json = Config();
    Config config = (backend_json.empty() ? config_json : backend_json);
    LOGGER_DEBUG("QPU config ready.");
    QPU qpu(std::make_unique<BackendType>(config, std::move(simulator)), mode, family);
    LOGGER_DEBUG("QPU instantiated.");
    qpu.turn_ON();
}

std::string generate_FakeQMIO(JSON back_path_json)
{
    std::string command("python "s + std::getenv("INSTALL_PATH") + "/cunqa/fakeqmio.py "s + back_path_json.at("fakeqmio_path").get<std::string>() + " "s +  std::getenv("SLURM_JOB_ID"));
    std::system(("ml load qmio/hpc gcc/12.3.0 qmio-tools/0.2.0-python-3.9.9 qiskit/1.2.4-python-3.9.9 2> /dev/null\n"s + command).c_str());
    return std::getenv("STORE") + "/.cunqa/tmp_fakeqmio_backend_"s + std::getenv("SLURM_JOB_ID") + ".json"s;
}

int main(int argc, char *argv[])
{
    std::string info_path(argv[1]);
    std::string mode(argv[2]);
    std::string communications(argv[3]);
    std::string family(argv[4]);
    std::string sim_arg(argv[5]);
    auto back_path_json = (argc == 7 ? JSON::parse(std::string(argv[6]))
                                     : JSON());

    JSON backend_json;
    if (back_path_json.contains("fakeqmio_path")) {
        std::ifstream f(generate_FakeQMIO(back_path_json));
        backend_json = JSON::parse(f);
    } else if (back_path_json.contains("backend_path")) {
        std::ifstream f(back_path_json.at("backend_path").get<std::string>());
        backend_json = JSON::parse(f);
    }

    if (family == "default")
        family = std::getenv("SLURM_JOB_ID");

    switch(murmur::hash(communications)) {
        case murmur::hash("no_comm"): 
        LOGGER_DEBUG("Raising QPU without communications.");
            switch(murmur::hash(sim_arg)) {
                case murmur::hash("Aer"): 
                    turn_ON_QPU<AerSimpleSimulator, SimpleConfig, SimpleBackend>(backend_json, mode, family);
                    LOGGER_DEBUG("QPU turned on with AerSimpleSimulator.");
                    break;
                case murmur::hash("Munich"):
                    turn_ON_QPU<MunichSimpleSimulator, SimpleConfig, SimpleBackend>(backend_json, mode, family);
                    LOGGER_DEBUG("QPU turned on with MunichSimpleSimulator.");
                    break;
                case murmur::hash("Cunqa"):
                    turn_ON_QPU<CunqaSimpleSimulator, SimpleConfig, SimpleBackend>(backend_json, mode, family);
                    LOGGER_DEBUG("QPU turned on with CunqaSimpleSimulator.");
                    break;
                default:
                    LOGGER_ERROR("Simulator {} do not support simple simulation or does not exist.", sim_arg);
                    return EXIT_FAILURE;
            }
        case murmur::hash("classical_comm"): 
        LOGGER_DEBUG("Raising QPU with classical communications.");
            switch(murmur::hash(sim_arg)) {
                case murmur::hash("Cunqa"): 
                    turn_ON_QPU<CunqaCCSimulator, SimpleConfig, SimpleBackend>(backend_json, mode, family);
                    LOGGER_DEBUG("QPU turned on with CunqaCCSimulator.");
                    break;
                default:
                    LOGGER_ERROR("Simulator {} do not support classical communication simulation or does not exist.", sim_arg);
                    return EXIT_FAILURE;
            }
        default:
            LOGGER_ERROR("No {} communication method available.", communications);
            return EXIT_FAILURE;
    }     
    return EXIT_SUCCESS;
}