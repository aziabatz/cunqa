
#include "quantum_task.hpp"

namespace cunqa {

void QuantumTask::update_circuit(const std::string& quantum_task) 
{
    auto quantum_task_json = JSON::parse(quantum_task);

    if (quantum_task_json.contains("instructions") && quantum_task_json.contains("config")) {
        circuit = quantum_task_json.at("instructions");
        config = quantum_task_json.at("config");
    } else if (quantum_task_json.contains("params")) {
        upgrade_params_(quantum_task_json.at("params");)
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
            switch(CUNQA::INSTRUCTIONS_MAP[name]){
                case CUNQA::MEASURE:
                case CUNQA::ID:
                case CUNQA::X:
                case CUNQA::Y:
                case CUNQA::Z:
                case CUNQA::H:
                case CUNQA::CX:
                case CUNQA::CY:
                case CUNQA::CZ:
                    break;
                case CUNQA::RX:
                case CUNQA::RY:
                case CUNQA::RZ:
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