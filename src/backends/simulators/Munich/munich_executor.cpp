#include <fstream>
#include <iostream>

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
    std::string endpoint;
    for (int i=0; i<=n_qpus; i++) {
        std::getline(in, endpoint);
        this->classical_channel.connect(endpoint);
    }
    
    if (in.bad()) {
        LOGGER_ERROR("Error de E/S al leer el fichero: {}", filename);
    }

    in.close();
    return "";
}

void MunichExecutor::run()
{
    while (true) {
        for(const auto& qpu_ids: qpu_id) {
            this->classical_channel->recv(qpu_id)
        }
    }
    
    
}


} // End of sim namespace
} // End of cunqa namespace