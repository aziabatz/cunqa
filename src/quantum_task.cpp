#include "quantum_task.hpp"
#include "utils/constants.hpp"

#include "logger.hpp"

namespace cunqa {

void QuantumTask::update_circuit(const std::string& quantum_task) 
{
    auto quantum_task_json = JSON::parse(quantum_task);
    std::vector<std::string> no_communications = {};
    int aux_is_distributed;

    if (quantum_task_json.contains("instructions") && quantum_task_json.contains("config")) {
        circuit = quantum_task_json.at("instructions");
        config = quantum_task_json.at("config");
        sending_to = (quantum_task_json.contains("sending_to") ? quantum_task_json.at("sending_to").get<std::vector<std::string>>() : no_communications);
        is_distributed = ((quantum_task_json.contains("is_distributed")) ? quantum_task_json.at("is_distributed").get<bool>() : false);

    } else if (quantum_task_json.contains("params")) {
        update_params_(quantum_task_json.at("params"));
    } else
        throw std::runtime_error("Incorrect format of the circuit.");
}

    
void QuantumTask::update_params_(const std::vector<double> params)
{
    if (circuit.empty()) 
        throw std::runtime_error("Circuit not sent before updating parameters.");

    try{
        int counter = 0;
        
        for (auto& instruction : circuit){

            std::string name = instruction.at("name");
            
            
            // TODO: Look at the instructions constants and how to work with them
            switch(cunqa::constants::INSTRUCTIONS_MAP.at(name)){
                case cunqa::constants::MEASURE:
                case cunqa::constants::ID:
                case cunqa::constants::X:
                case cunqa::constants::Y:
                case cunqa::constants::Z:
                case cunqa::constants::H:
                case cunqa::constants::CX:
                case cunqa::constants::CY:
                case cunqa::constants::CZ:
                    break;
                case cunqa::constants::RX:
                case cunqa::constants::RY:
                case cunqa::constants::RZ:
                    instruction.at("params")[0] = params[counter];
                    counter = counter + 1;
                    break; 
            }
        }
        
    } catch (const std::exception& e){
        LOGGER_ERROR("Error updating parameters. (check correct size).");
        throw std::runtime_error("Error updating parameters:" + std::string(e.what())); 
    }
}

} // End of cunqa namespace