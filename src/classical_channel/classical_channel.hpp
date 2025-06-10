#pragma once

#include <string>
#include <vector>
#include <memory>

namespace cunqa {
namespace comm {

class ClassicalChannel {
public:

    ClassicalChannel();
    ~ClassicalChannel();

    void set_classical_connections(std::vector<std::string>& qpus_id);
    void send_measure(int& measurement, std::string& target);
    int recv_measure(std::string& origin);
    

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
public:
    std::string endpoint;
};  

} // End of comm namespace
} // End of cunqa namespace