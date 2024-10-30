#include <iostream>
#include <memory>
#include "../utils/ip_config.hpp"

#if COMM_LIB == ASIO
    #include "strategies/asio/asio_server.hpp"
    using SelectedServer = AsioServer;
#elif COMM_LIB == ZMQ
    #include "strategies/zmq/zmq_comm.hpp"
    using SelectedServer = ZMQServer;
#elif COMM_LIB == CROW
    #include "strategies/crow/crow_comm.hpp"
    using SelectedServer = CrowServer;
#else
    #error "A valid library should be defined (ASIO, ZMQ o CROW) in COMM_LIB."
#endif

class Server {
    std::unique_ptr<SelectedServer> strategy;
public:

    Server(const IPConfig& ip_config) :
    strategy{std::make_unique<SelectedServer>(ip_config)} { }

    void start() {
        strategy->start();
    }

    void stop() {
        strategy->stop();
    }
    /* void send(const std::string& message) {
        strategy->sendMessage(message);
    }

    std::string receive() {
        return strategy->receiveMessage();
    }
 */
private:



};

