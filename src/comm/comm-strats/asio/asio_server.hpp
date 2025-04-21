#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include "utils/helpers.hpp"
#include "utils/constants.hpp"
#include "logger/logger.hpp"
#include "config/net_config.hpp"
#include "logger/logger.hpp"

namespace as = boost::asio;
using namespace config;
using namespace std::string_literals;
using as::ip::tcp;

class AsioServer {
public:
    AsioServer(const NetConfig& net_config) 
        : net_config_{net_config},
        io_context_{},
        acceptor_(
            io_context_,
            tcp::endpoint{
                as::ip::address::from_string(
                    net_config_.IPs.at(
                        std::string(net_config_.mode == "cloud" ? INFINIBAND : LOCAL)
                    )
                ),
                static_cast<unsigned short>(std::stoul(net_config_.port))
            }
        ),
        socket_{io_context_}  
    {
        acceptor_.accept(socket_);
    }

    void accept()
    {
        acceptor_.accept(socket_);
    }

    std::string recv_data() 
    { 
        try {
            uint32_t data_length_network;
            as::read(socket_, as::buffer(&data_length_network, sizeof(data_length_network)));
            
            uint32_t data_length = ntohl(data_length_network);
            std::string data(data_length, '\0');
            as::read(socket_, as::buffer(&data[0], data_length));
            return data;
        } catch (const boost::system::system_error& e) {
            if (e.code() == as::error::eof) {
                SPDLOG_LOGGER_DEBUG(logger, "Client disconnected, closing conection.");
                socket_.close(); 
                return std::string("CLOSE");
            } else {
                SPDLOG_LOGGER_ERROR(logger, "Error receiving the circuit.");
                throw;
            }
        }

        return std::string();
    }

    inline void send_result(const std::string& result) 
    {
        try {    
            auto data_length = legacy_size_cast<uint32_t, std::size_t>(result.size());
            auto data_length_network = htonl(data_length);

            as::write(socket_, as::buffer(&data_length_network, sizeof(data_length_network))); 
            as::write(socket_, as::buffer(result));
        } catch (const boost::system::system_error& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error sending the result.");
            throw;
        }
    }

    inline void close()
    {
        this->socket_.close();
    }

private:
    NetConfig net_config_;
    as::io_context io_context_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
};
