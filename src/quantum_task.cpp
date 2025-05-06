#include "quantum_task.hpp"
#include "utils/constants.hpp"

#include "logger.hpp"

namespace cunqa {

void QuantumTask::update_circuit(const std::string& quantum_task) 
{
    auto quantum_task_json = JSON::parse(quantum_task);

    if (quantum_task_json.contains("instructions") && quantum_task_json.contains("config")) {
        circuit = quantum_task_json.at("instructions");
        config = quantum_task_json.at("config");
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
            switch(INSTRUCTIONS_MAP.at(name)){
                case MEASURE:
                case ID:
                case X:
                case Y:
                case Z:
                case H:
                case CX:
                case CY:
                case CZ:
                    break;
                case RX:
                case RY:
                case RZ:
                    instruction.at("params")[0] = params[counter];
                    counter = counter + 1;
                    break; 
            }
        }
        
    } catch (const std::exception& e){
        LOGGER_ERROR("Error updating parameters. (check correct size).");
        throw std::runtime_error("Error updating parameters.");
    }
}

} // End of cunqa namespace