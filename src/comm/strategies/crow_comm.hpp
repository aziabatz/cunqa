#include "base_comm.hpp"

class CrowComm : public BaseComm {
public:
    void startServer() override {
        // Código específico de Crow
    }
    void sendMessage(const std::string& message) override {
        // Envío de mensaje con Crow
    }
    std::string receiveMessage() override {
        // Recepción de mensaje con Crow
        return "message";
    }
};