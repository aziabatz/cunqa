#include <iostream>
#include <fstream>
#include <mpi.h>
#include <exception>
#include <sys/file.h>
#include <unistd.h>

#include "json.hpp"
#include "utils/helpers/runtime_env.hpp"

namespace cunqa {

void write_on_file(JSON local_data, const std::string &filename, const std::string& suffix) 
{
    try {
        int file = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
        if (file == -1) {
            std::cerr << "Error al abrir el archivo" << std::endl;
            return;
        }
        flock(file, LOCK_EX);

        JSON j;
        std::ifstream file_in(filename);

        if (file_in.peek() != std::ifstream::traits_type::eof())
            file_in >> j;
        file_in.close();

        // These variables identify the process within the job
        std::string local_id = runtime_env::task_pid();
        std::string job_id = runtime_env::job_id();
        auto task_id = (suffix == "") ? job_id + "_" + local_id : job_id + "_" + local_id + "_" + suffix;
        
        j[task_id] = local_data;

        std::ofstream file_out(filename, std::ios::trunc);
        file_out << j.dump(4);
        file_out.close();

        flock(file, LOCK_UN);
        close(file);
    } catch(const std::exception& e) {
        std::string msg("Error writing the JSON simultaneously using locks.\nError message thrown by the system: "); 
        throw std::runtime_error(msg + e.what());
    }
}

}
