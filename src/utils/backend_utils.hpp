#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string install_path = std::getenv("INSTALL_PATH");

namespace basis_gates {

    json basis_gates(std::string path_to_basis_gates_json = install_path + "/include/utils/basis_gates.json")
    {
        json gates_json;
        std::string path = path_to_basis_gates_json;

        std::ifstream gates_file(path);

        gates_file >> gates_json;
        
        gates_file.close();


        return gates_json;
    }

}


