#pragma once

#include <string>
#include <regex>

bool check_time_format(const std::string& time)
{
    std::regex format("^(\\d{2}):(\\d{2}):(\\d{2})$");
    return std::regex_match(time, format);   
}

bool check_mem_format(const std::string& mem) 
{
    std::regex format("^(\\d{1,2})G$");
    return std::regex_match(mem, format);
}

int check_memory_specs(int& mem_per_qpu, int& cores_per_qpu)
{
    int mem_per_cpu = mem_per_qpu/cores_per_qpu;
    const char* system_var = std::getenv("LMOD_SYSTEM_NAME");
    if ((std::string(system_var) == "QMIO" && mem_per_cpu > 15)) {
        return 1;
    } else if ((std::string(system_var) == "FT3" && mem_per_cpu > 4)){
        return 2;
    }

    SPDLOG_LOGGER_DEBUG(logger, "Correct memory per core.");

    return 0;
}