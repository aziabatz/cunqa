#pragma once

#include <iostream>
#include <memory>
#include <queue>
#include <string>

namespace cunqa {
namespace comm {

class ServerException : public std::exception {
    std::string message;
public:
    explicit ServerException(const std::string& msg) : message(msg) { }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class Server {
public:
    std::string mode;
    std::string hostname;
    std::string nodename;
    std::string ip;
    std::string port;

    Server(const std::string& mode);
    ~Server();

    void accept();
    std::string recv_data();
    void send_result(const std::string& result);
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;

    friend void to_json(JSON &j, SimpleBackend obj) {
        //
    }

    friend void from_json(JSON j, SimpleBackend &obj) {
        //
    }
};

} // End of comm namespace
} // End of cunqa namespace

