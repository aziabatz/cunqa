#pragma once

#include <nlohmann/json.hpp>

namespace cunqa {

    using JSON = nlohmann::json;

    [[deprecated("Unused because iterference with MPI for classical/quantum communications")]]
    void write_MPI(JSON local_data, const std::string &filename);
    
    void write_on_file(JSON local_data, const std::string &filename);
}

