#include <boost/asio.hpp>
#include "../../../utils/ip_config.hpp"
#include "asio_common.hpp"

using boost::asio::ip::tcp;

class AsioClient {
    boost::asio::io_context io_context;
    std::shared_ptr<tcp::socket> socket;

public:

    AsioClient() :
        socket{std::make_shared<tcp::socket>(io_context)} 
    { }
    
    void connect(const IPConfig& server_ipconfig) 
    {
        tcp::resolver resolver{io_context};
        auto endpoint = resolver.resolve(server_ipconfig.hostname, server_ipconfig.port);
        boost::asio::connect(*socket, endpoint);
    }

    void send_data(const std::string& data) 
    {
        send_string(socket, data);
    }

    void send_data(std::ifstream& file) 
    {
        std::string data((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()) );
        send_string(socket, data);
    }

    void read_result() 
    {
        auto result = read_string(*socket);
    }

    void stop() 
    {
        boost::system::error_code ec;
        socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket->close();
    }

};