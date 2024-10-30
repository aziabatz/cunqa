#include <boost/asio.hpp>
#include "../../../utils/ip_config.hpp"

using boost::asio::ip::tcp;

class AsioClient {
    boost::asio::io_context io_context;
    tcp::socket socket;
    tcp::resolver resolver;

public:

    AsioClient() :
        socket{io_context},
        resolver{io_context} {}
    
    void connect(const IPConfig& server_ipconfig) {
        auto endpoint = resolver.resolve(server_ipconfig.hostname, server_ipconfig.port);
        boost::asio::connect(socket, endpoint);
    }

    void write(const std::string& message) {
        auto result = boost::asio::write(socket, boost::asio::buffer(message));

        std::cout << "data sent: " << message.length() << '/' << result << "\n";
    }

    void read() {
        boost::system::error_code error;
        std::string response;
        boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), '\n', error);
        
        if (!error) {
            std::cout << "Response from server: " << response << "\n";
        } else 
             std::cout << "Error in response\n";
    }

    void stop() {
        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close();
    }

};