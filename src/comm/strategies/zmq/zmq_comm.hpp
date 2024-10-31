#include "base_comm.hpp"

class ZMQComm : public BaseComm {
public:
    void startServer() override {
        // TODO
    }
    void sendMessage(const std::string& message) override {
        // TODO
    }
    std::string receiveMessage() override {
        // TODO
        return "message";
    }
};
