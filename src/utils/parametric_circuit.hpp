#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

#include <regex>
#include <sstream>

#include "constants.hpp"
#include "logger/logger.hpp"

using json = nlohmann::json;

json empty_json = {};
std::string empty_string = "";

//TODO: QASM
std::vector<double> get_circuit_parameters(const json& circuit = empty_json, std::string& qasm = empty_string)
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

        return circuit;
        
    } catch (const std::exception& e){
        SPDLOG_LOGGER_ERROR(logger, "Error updating parameters. (check correct size).");
        throw std::runtime_error("Error updating parameters.");
    }
}


json update_qasm_parameters(json& circuit, const std::vector<double>& params) {

    if (!circuit.contains("instructions")) {
        throw std::runtime_error("Invalid circuit format, circuit must have an instruction field.");
    }

    try{

        std::string qasmCode = circuit.at("instructions");
        std::regex paramRegex(R"((rx|ry|rz|u1|u2|u3)\(\s*([0-9]*\.?[0-9]+)\s*\))");  // Match gate parameters
        std::smatch match;

        std::string updatedQasm = qasmCode;
        size_t paramIndex = 0;

        auto words_begin = std::sregex_iterator(qasmCode.begin(), qasmCode.end(), paramRegex);
        auto words_end = std::sregex_iterator();

        // Check if the number of parameters match
        size_t numParamsFound = std::distance(words_begin, words_end);
        if (numParamsFound != params.size()) {
            throw std::runtime_error("Number of parameters in QASM does not match provided values.");
        }

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::string oldValue = (*i)[2].str();  // Extract old numeric value
            std::string newValue = std::to_string(params[paramIndex++]);

            // Replace the found value in the QASM string
            updatedQasm = std::regex_replace(updatedQasm, std::regex(R"(\b)" + oldValue + R"(\b)"), newValue, std::regex_constants::format_first_only);
        }

        circuit.at("instructions") = updatedQasm;

        return circuit;

    } catch (const std::exception& e) {
        SPDLOG_LOGGER_ERROR(logger, "Error updating qasm parameters. (check correct size).");
        throw std::runtime_error("Error updating qasm parameters.");
    }
}