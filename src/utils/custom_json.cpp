#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <exception>

#include "custom_json.hpp"

CustomJson::CustomJson() { 
    custom_json = json::object();
}
CustomJson::~CustomJson() { }

void CustomJson::write(json data) {
    int flag;
    MPI_Initialized(&flag); 
    flag ? CustomJson::_write_MPI(data) : CustomJson::_write_locks(data);
}

// TODO: Improve the filename stablishing a tmp directory for the json files
void CustomJson::dump(const std::string& filename){
    try {
        std::fstream file(filename, std::ios::out);
        file << custom_json.dump(4) << "\n";
    } catch(std::exception& e){
        std::cerr << "Error, cannot open JSON file: " << e.what() << std::endl;
    }
}

const std::string CustomJson::dump(){
    return custom_json.dump(4);
}

void CustomJson::_write_MPI(json local_data) {
        int world_rank, world_size;
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); 
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        // Prepare all the sending data
        std::string local_data_str;
        int local_length;
        const char* send_data;
        local_data_str = local_data.dump();
        local_length = local_data_str.size();
        send_data = local_data_str.c_str();

        // Gather lengths to calculate the displacements
        int* recv_lengths;
        if (world_rank == 0) recv_lengths = new int[world_size];
        MPI_Gather(&local_length, 1, MPI_INT, recv_lengths, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Calculate displacements at root
        int* displs;
        int total_length = 0;
        if (world_rank == 0) {
            
            displs = new int[world_size];
            displs[0] = 0;
            for (int i = 0; i < world_size; ++i) {
                if (i > 0) displs[i] = displs[i - 1] + recv_lengths[i - 1];
                total_length += recv_lengths[i];
            }
        }

        // Gather variable-length data at root
        char* recv_data;
        if (world_rank == 0) recv_data = new char[total_length];

        MPI_Gatherv(send_data, local_length, MPI_CHAR,
                    recv_data, recv_lengths, displs, MPI_CHAR,
                    0, MPI_COMM_WORLD);

        // Reconstruct strings at root
        if (world_rank == 0) {    
            std::vector<json> all_json_data(world_size);
            for (int i = 0; i < world_size; ++i) {
                std::string data_str(recv_data + displs[i], recv_lengths[i]);
                all_json_data[i] = json::parse(data_str);
            }

            for (const auto& obj : all_json_data) {
                custom_json.update(obj);
            }
        }
    }

void CustomJson::_write_locks(json local_data) {
        // TODO: Write the JSON file with locks if the MPI environment
    }
