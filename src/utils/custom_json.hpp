#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class CustomJson {
    json custom_json;
public:
    CustomJson();
    ~CustomJson();

    void write(json data);    
    void dump(const std::string& filename);
    const std::string dump();
private:
    void _write_MPI(json data);
    void _write_locks(json local_data);
};