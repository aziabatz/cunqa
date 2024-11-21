#include <boost/asio.hpp>
#include "../../ip_config.hpp"
#include "../../../utils/constants.hpp"
#include "asio_common.hpp"
#include <string>

using boost::asio::ip::tcp;
using namespace std::literals;

class AsioClient {
    boost::asio::io_context io_context;
    std::shared_ptr<tcp::socket> socket;

public:

    AsioClient() :
        socket{std::make_shared<tcp::socket>(io_context)} 
    { }
    
    inline void connect(const ip_config::IPConfig& server_ipconfig, const std::string_view& net = INFINIBAND) 
    {
        tcp::resolver resolver{io_context};
        // TODO: probar con el enpoint del IPconfig directamente
        auto endpoint = resolver.resolve(server_ipconfig.IPs.at(std::string(net)), server_ipconfig.port);
        boost::asio::connect(*socket, endpoint);
    }

    inline void send_data(const std::string& data) 
    {
        send_string(socket, data);
    }

    inline void send_data(std::ifstream& file) 
    {
        std::string data((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()) );
        send_string(socket, data);
    }

    inline void read_result() 
    {
        auto result = read_string(*socket);
    }

    inline void stop() 
    {
        boost::system::error_code ec;
        socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket->close();
    }

};