#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

#include "constants.hpp"

using json = nlohmann::json;


std::vector<double> get_circuit_parameters(const json& circuit)
{
    if (!circuit.contains("instructions"))
        throw std::runtime_error("Invalid circuit format, circuit must have an instruction field.");

    std::vector<double> list_params = {};
    for (auto& instruction : circuit.at("instructions")){

        std::string name = instruction.at("name");
        switch(GATE_NAMES[name]){
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
                list_params.push_back(instruction.at("params")[0]);
                break;
        }
    }
    
    return list_params;
}


//TODO: Two-qubit gates with parameters. params = list of lists
json update_circuit_parameters(json& circuit, const std::vector<double>& params)
{
    if (!circuit.contains("instructions")) 
        throw std::runtime_error("Invalid circuit format, circuit must have an instruction field.");

    try{
        int counter = 0;
        for (auto& instruction : circuit.at("instructions")){

            std::string name = instruction.at("name");

            switch(GATE_NAMES[name]){
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
        SPDLOG_LOGGER_ERROR(logger, "Error updating parameters. (check correct size).");
        throw std::runtime_error("Error updating parameters.");
    }

    return circuit;
}