#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "../../../utils/helpers.hpp"
#include "asio_common.hpp"

using namespace std::string_literals;
using boost::asio::ip::tcp;

class AsioServer {
    IPConfig ip_config;
    boost::asio::io_context io_context;
    std::shared_ptr<tcp::socket> socket;
    tcp::acceptor acceptor;

public:
    AsioServer(const IPConfig& ip_config) :
        ip_config{ip_config}, 
        acceptor{io_context, tcp::endpoint(tcp::v4(), std::stoi(ip_config.port))} 
    { }

    void start() 
    { 
        _start_accept(); 
        io_context.run();
    }

    void stop() { io_context.stop(); }

private:

    void _start_accept() {
        socket = std::make_shared<tcp::socket>(acceptor.get_executor());
        acceptor.async_accept(*socket, [this](const boost::system::error_code& error) {
            if (!error) {
                std::cout << "Client connected: " << socket->remote_endpoint() << std::endl;
                auto result = read_string(*socket);
                send_result(result);
            } else {
                std::cerr << "Accept error: " << error.message() << std::endl;
            }
            _start_accept();
        });
    }

    void send_result(const std::string& result) {
        send_string(socket, result);    
    }
};
