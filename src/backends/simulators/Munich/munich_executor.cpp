#include <fstream>
#include <iostream>

#include "munich_adapters/circuit_simulator_adapter.hpp"
#include "munich_adapters/quantum_computation_adapter.hpp"
#include "quantum_task.hpp"
#include "munich_executor.hpp"

#include "utils/json.hpp"
#include "logger.hpp"

namespace cunqa {
namespace sim {

void MunichExecutor(const int& n_qpus)

    const char* store = std::getenv("STORE");
    std::string filename = std::string(store) + "/endpoints_" + std::getenv("SLURM_JOB_ID");

    std::ifstream in(filename);

    if (!in.is_open()) {
        LOGGER_ERROR("No se pudo abrir el fichero: {}", filename);
        return "";
    }

    //Esto va a cambiar
    std::string endpoint, id;
    for (int i=0; i<=n_qpus; i++) {
        // Aquí leemos el json y 
        //std::getline(in, endpoint);
        this->classical_channel.connect(endpoint, id);
    }
    
    if (in.bad()) {
        LOGGER_ERROR("Error de E/S al leer el fichero: {}", filename);
    }

    in.close();
    return "";
}

void MunichExecutor::run()
{
    std::vector<QuantumTask> quantum_tasks;
    while (true) {
        for(const auto& qpu_ids: qpu_id) {
            JSON quantum_task_json = JSON::parse(this->classical_channel->recv_info(qpu_id));
            quantum_tasks.push_back(QuantumTask(quantum_task_json.at(0), quantum_task_json.at(1)));
        }

        auto qc = std::make_unique<QuantumComputationAdapter>(quantum_tasks);
        CircuitSimulatorAdapter simulator(std::move(qc));

        // TODO: Mirar como hacer lo de los shots
        auto start = std::chrono::high_resolution_clock::now();
        auto result = simulator.simulate(1024);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start;
        time_taken = duration.count();
        
        // TODO: transformar los circuitos 
        std::string result_str = "prueba";

        this->classical_channel.send_info()

        quantum_tasks.clear();
    }
    
    
}


} // End of sim namespace
} // End of cunqa namespace