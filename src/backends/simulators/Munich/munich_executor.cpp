#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

#include "munich_adapters/circuit_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"
#include "quantum_task.hpp"
#include "munich_executor.hpp"

#include "utils/json.hpp"
#include "logger.hpp"

namespace cunqa {
namespace sim {

MunichExecutor::MunichExecutor() : classical_channel{"executor"}
{
    std::string filename = std::string(std::getenv("STORE")) + "/.cunqa/communications.json";
    std::ifstream in(filename);

    if (!in.is_open()) {
        LOGGER_ERROR("Could not open the file: {}", filename);
        throw std::runtime_error("Error opening the communications file.");
    }

    JSON j;
    if (in.peek() != std::ifstream::traits_type::eof())
        in >> j;
    in.close();

    LOGGER_DEBUG("JSON of communications: \n{}", j.dump(4));

    std::string job_id = std::getenv("SLURM_JOB_ID");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    for (const auto& [key, value]: j.items()) {
        if (key.rfind(job_id, 0) == 0) {
            auto qpu_endpoint = value["communications_endpoint"].get<std::string>();
            qpu_ids.push_back(qpu_endpoint);
            classical_channel.connect(qpu_endpoint);
            LOGGER_DEBUG("Sending my endpoint: {} to {}", classical_channel.endpoint, qpu_endpoint);
            classical_channel.send_info(classical_channel.endpoint, qpu_endpoint);
        }
    }
}

void MunichExecutor::run()
{
    std::vector<QuantumTask> quantum_tasks;
    JSON quantum_task_json;
    std::vector<std::string> qpus_working;
    std::string message;
    while (true) {
        for(const auto& qpu_id: qpu_ids) {
            message = classical_channel.recv_info(qpu_id);
            if(message != "") {
                qpus_working.push_back(qpu_id);
                quantum_task_json = JSON::parse(message);
                quantum_tasks.push_back(QuantumTask(quantum_task_json["instructions"], quantum_task_json["config"]));
            }
        }

        auto qc = std::make_unique<QuantumComputationAdapter>(quantum_tasks);
        CircuitSimulatorAdapter simulator(std::move(qc));

        // TODO: Mirar como hacer lo de los shots
        auto start = std::chrono::high_resolution_clock::now();
        auto result = simulator.simulate(1024);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start;
        float time_taken = duration.count();
        
        // TODO: transformar los circuitos 
        std::string result_str = "prueba";

        for(const auto& qpu: qpus_working){
            classical_channel.send_info(result_str, qpu);
        }

        qpus_working.clear();
        quantum_tasks.clear();
    }
}


} // End of sim namespace
} // End of cunqa namespace