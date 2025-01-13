#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include "utils/helpers.hpp"
#include "utils/constants.hpp"
#include "config/net_config.hpp"

namespace as = boost::asio;
using namespace config;
using namespace std::string_literals;
using as::ip::tcp;

class AsioServer {
    NetConfig net_config;
public:
    AsioServer(const NetConfig& net_config, const std::string_view& net = INFINIBAND) :
        net_config{net_config},
        acceptor_{io_context_, tcp::endpoint{as::ip::address::from_string(net_config.IPs.at(std::string(net))), 
                            static_cast<unsigned short>(stoul(net_config.port))}},
        socket_{acceptor_.get_executor()}
    { 
        acceptor_.accept(socket_);
    }

    void accept()
    {
        acceptor_.accept(socket_);
    }

    std::string recv_data() 
    { 
        // Receive string size
        uint32_t data_length_network;
        boost::system::error_code ec;
        as::read(socket_, as::buffer(&data_length_network, sizeof(data_length_network)), ec);
        
        if (!ec) {
            uint32_t data_length = ntohl(data_length_network);
            std::string data(data_length, '\0');
            as::read(socket_, as::buffer(&data[0], data_length));

            return data;
        } else if (ec == as::error::eof) {
            std::cout << "Client disconnected, closing conection.\n";
            socket_.close();

            return std::string("CLOSE");
        } else {
            // Otros errores
            std::cerr << "Error en la lectura: " << ec.message() << std::endl;
        }

        return std::string();
    }

    inline void send_result(const std::string& result) {
        auto data_length = legacy_size_cast<uint32_t, std::size_t>(result.size());
        auto data_length_network = htonl(data_length);

        as::write(socket_, as::buffer(&data_length_network, sizeof(data_length_network))); 
        as::write(socket_, as::buffer(result)); 
    }

private:
    as::io_context io_context_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
};
