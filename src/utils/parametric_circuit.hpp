#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

#include "constants.hpp"

using json = nlohmann::json;


std::vector<double> get_circuit_parameters(json& circuit)
{

    std::vector<double> list_params = {};

    if (!circuit.contains("instructions")){
        std::cout << "Error. Invalid circuit format" << "\n";
        return list_params;
    }
    
    std::vector<json> circuit_instructions = circuit["instructions"]; 
    

    for (auto& instruction : circuit_instructions){

        std::string name = instruction["name"];

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
                list_params.push_back(instruction["params"][0]);
                break;

        }

    }

    return list_params;

}


//TODO: Two-qubit gates with parameters. params = lista de listas
json update_circuit_parameters(json& circuit, std::vector<double> params)
{


    if (!circuit.contains("instructions")){
        std::cout << "Error. Invalid circuit format" << "\n";
        return circuit;
    }
    

    int counter = 0;

    for (auto& instruction : circuit["instructions"]){

        std::string name = instruction["name"];

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
                instruction["params"][0] = params[counter];
                counter = counter + 1;
                break;
            
        } 

    }



    return circuit;

}