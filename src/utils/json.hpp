#pragma once

#include <cerrno>
#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <sys/file.h>
#include <unistd.h>

#include <nlohmann/json.hpp>

namespace cunqa {
    
using JSON = nlohmann::json;
inline void write_on_file(JSON local_data, const std::string& filename, const std::string& suffix = "")
{
    try {
        const int file = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
        if (file == -1) {
            std::cerr << "Error al abrir el archivo" << std::endl;
            return;
        }
        flock(file, LOCK_EX);
        JSON j;
        std::ifstream file_in(filename);
        if (file_in.peek() != std::ifstream::traits_type::eof()) {
            file_in >> j;
        }
        file_in.close();
        // SLURM variables conforming the process identifier
        const char* local_id_env = std::getenv("SLURM_TASK_PID");
        const char* job_id_env = std::getenv("SLURM_JOB_ID");
        const std::string local_id = local_id_env ? local_id_env : "";
        const std::string job_id = job_id_env ? job_id_env : "";
        const std::string task_id = suffix.empty() ? job_id + "_" + local_id
                                                   : job_id + "_" + local_id + "_" + suffix;
        j[task_id] = std::move(local_data);
        std::ofstream file_out(filename, std::ios::trunc);
        file_out << j.dump(4);
        file_out.close();
        flock(file, LOCK_UN);
        close(file);
    } catch (const std::exception& e) {
        std::string msg("Error writing the JSON simultaneously using locks.\nError message thrown by the system: ");
        throw std::runtime_error(msg + e.what());
    }
}
} // namespace cunqa