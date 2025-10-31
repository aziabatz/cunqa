#pragma once

#include <string>
#include <regex>
#include <fstream>
#include <cmath>
#include <cstdio> // For popen, pclose

#include "utils/json.hpp"
#include "logger.hpp"

bool check_time_format(const std::string& time)
{
    std::regex format("^(\\d{2}):(\\d{2}):(\\d{2})$");
    return std::regex_match(time, format);   
}

bool check_mem_format(const int& mem) 
{
    std::string mem_str = std::to_string(mem) + "G";
    std::regex format("^(\\d{1,4})G$");
    
    return std::regex_match(mem_str, format);
}

int get_default_mem_per_core()
{
    
    if (SYSTEM_NAME == "QMIO") {
        return 15;
    } else if (SYSTEM_NAME == "FT3"){
        return 4;
    } else if (SYSTEM_NAME == "MY_CLUSTER") {
        return 2; // To be changed to the specific cluster
    }
    return 0;
}


bool exists_family_name(const std::string& family, const std::string& info_path)
{
    std::ifstream file(info_path);
    if (!file) {
        return false;
    } else {
        cunqa::JSON qpus_json;
        try {
            file >> qpus_json;
            for (auto& [key, value] : qpus_json.items()) {
                if (value["family"] == family) {
                    return true;
                } 
            }
            return false;
        } catch (const std::exception& e) {
            LOGGER_DEBUG("The qpus.json file was completely empty. An empty json will be written on it.");
            file.close();
            std::ofstream out(info_path);
            if (!out) {
                LOGGER_DEBUG("Impossible to open the empty qpus.json file to write on it. It will be deleted and created again");
                std::remove(info_path.c_str());
                out.open(info_path);
                return false;  
            }
            out << "{ }";  
            out.close();

            return false;
        }
    }
}

bool check_simulator_name(const std::string& sim_name)
{
    if (sim_name == "Cunqa" || sim_name == "Munich" || sim_name == "Aer") {  // Add new valid simulators to the check here
        return true;
    } else {
        return false;
    }
}

int number_of_nodes(const int& number_of_qpus, const int& cores_per_qpu, const int& number_of_nodes, const int& cores_per_node, const bool has_qc = false)
{
    int available_cores = number_of_nodes * cores_per_node;;
    int needed_cores = has_qc ? number_of_qpus * cores_per_qpu + number_of_qpus : number_of_qpus * cores_per_qpu;

    if (needed_cores < available_cores) {
        return number_of_nodes;
    } else {
        float aprox_number_of_nodes = needed_cores/(float)cores_per_node;
        return static_cast<int>(std::ceil(aprox_number_of_nodes));
    }
}

#ifdef CUNQA_SLURMLESS
inline std::string allocate_port_range(int count)
{
    static int next_port = 55000;
    if (count <= 0) {
        return {};
    }
    if (next_port + count >= 65000) {
        next_port = 55000;
    }
    int low = next_port;
    next_port += count + 10;
    int high = low + count - 1;
    return std::to_string(low) + "-" + std::to_string(high);
}
#endif
