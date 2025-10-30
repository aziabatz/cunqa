#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

#include "munich_adapters/munich_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"
#include "quantum_task.hpp"
#include "munich_executor.hpp"

#include "utils/json.hpp"
#include "logger.hpp"
#include "utils/helpers/runtime_env.hpp"

namespace cunqa {
namespace sim {

MunichExecutor::MunichExecutor() : classical_channel{"executor"}
{
    LOGGER_DEBUG("Vamos a inicializar el executor.");
    std::string filename = std::string(std::getenv("STORE")) + "/.cunqa/communications.json";
    std::ifstream in(filename);

    if (!in.is_open()) {
        throw std::runtime_error("Error opening the communications file.");
    }

    JSON j;
    if (in.peek() != std::ifstream::traits_type::eof())
        in >> j;
    in.close();

    std::string job_id = runtime_env::job_id();

    for (const auto& [key, value]: j.items()) {
        if (key.rfind(job_id, 0) == 0) {
            auto qpu_endpoint = value.at("communications_endpoint").get<std::string>();
            qpu_ids.push_back(qpu_endpoint);
            classical_channel.connect(qpu_endpoint);
            classical_channel.send_info(classical_channel.endpoint, qpu_endpoint);
        }
    }

    LOGGER_DEBUG("Executor perfectamente inicializado.");
};


MunichExecutor::MunichExecutor(const std::string& group_id) : classical_channel{"executor"}
{
    std::string filename = std::string(std::getenv("STORE")) + "/.cunqa/communications.json";
    std::ifstream in(filename);

    if (!in.is_open()) {
        throw std::runtime_error("Error opening the communications file.");
    }

    JSON j;
    if (in.peek() != std::ifstream::traits_type::eof())
        in >> j;
    in.close();

    for (const auto& [key, value]: j.items()) {
        if (key.rfind(group_id) == key.size() - group_id.size()) {
            auto qpu_endpoint = value["communications_endpoint"].get<std::string>();
            qpu_ids.push_back(qpu_endpoint);
            classical_channel.connect(qpu_endpoint);
            classical_channel.send_info(classical_channel.endpoint, qpu_endpoint);
        }
    }
};

void MunichExecutor::run()
{
    std::vector<QuantumTask> quantum_tasks;
    std::vector<std::string> qpus_working;
    JSON quantum_task_json;
    std::string message;
    while (true) {
        for(const auto& qpu_id: qpu_ids) {
            message = classical_channel.recv_info(qpu_id);
            if(!message.empty()) {
                qpus_working.push_back(qpu_id);
                quantum_task_json = JSON::parse(message);
                quantum_tasks.push_back(QuantumTask(message));
            }
        }

        auto qc = std::make_unique<QuantumComputationAdapter>(quantum_tasks);
        CircuitSimulatorAdapter simulator(std::move(qc));
        LOGGER_DEBUG("Vamos a llamar al simulate del adapter");
        auto result = simulator.simulate(&classical_channel);
        
        // TODO: transform results to give each qpu its results
        std::string result_str = result.dump();

        for(const auto& qpu: qpus_working) {
            classical_channel.send_info(result_str, qpu);
        }

        qpus_working.clear();
        quantum_tasks.clear();
    }
}


} // End of sim namespace
} // End of cunqa namespace
