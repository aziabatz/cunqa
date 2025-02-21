#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "config/net_config.hpp"
#include "utils/constants.hpp"
#include "utils/helpers.hpp"
#include "logger/logger.hpp"

namespace as = boost::asio;
using as::ip::tcp;
using namespace std::literals;
using namespace config;

class AsioClient;

class AsioFuture {
public:
    AsioFuture(AsioClient *client);
    std::string get();
    inline bool valid();
private:
    AsioClient *client;
};

class AsioClient {
public:
    friend class AsioFuture;

    AsioClient() :
        io_context_{},
        socket_{io_context_}
    { }

    ~AsioClient() 
    {
        socket_.close();
        io_context_.stop();
    }

    void connect(const NetConfig& server_ipconfig, const std::string_view& net = INFINIBAND) 
    {   
        try {
            tcp::resolver resolver{io_context_};
            auto endpoint = resolver.resolve(server_ipconfig.IPs.at(std::string(net)), server_ipconfig.port);
            as::connect(socket_, endpoint);
            SPDLOG_LOGGER_DEBUG(logger, "Client succesfully connected to server.");
        } catch (const boost::system::system_error& e) {
            SPDLOG_LOGGER_ERROR(logger,"Imposible to connect to endpoint {}:{}. Server not available.", server_ipconfig.IPs.at(std::string(net)), server_ipconfig.port);
        }
    }

    AsioFuture submit(const std::string& circuit) 
    {   
        _send_data(circuit);
        return AsioFuture(this);
    }

private:

    void _send_data(const std::string& data) 
    {
        auto data_length = legacy_size_cast<uint32_t, std::size_t>(data.size());
        auto data_length_network = htonl(data_length);

        try {
            as::write(socket_, as::buffer(&data_length_network, sizeof(data_length_network))); 
            as::write(socket_, as::buffer(data));
            SPDLOG_LOGGER_DEBUG(logger, "Message sent.");
            //SPDLOG_LOGGER_DEBUG(logger, "Message sent: ", data);
        } catch (const boost::system::system_error& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error sending the circuit.");
        }
        
    }

    std::string _recv_result() 
    {
        try {
            uint32_t result_length_network;
            as::read(socket_, as::buffer(&result_length_network, sizeof(result_length_network)));
            uint32_t result_length = ntohl(result_length_network);

            std::string result(result_length, '\0');
            as::read(socket_, as::buffer(&result[0], result_length));
            SPDLOG_LOGGER_DEBUG(logger, "Result correctly revceived.");
            SPDLOG_LOGGER_DEBUG(logger, "Result received: {}", result);
            return result;
        } catch (const boost::system::system_error& e) {
            SPDLOG_LOGGER_ERROR(logger, "Error receiving the circuit: {} (HINT: Check the circuit format and/or if QPUs are still up working.)", e.code().message());
        }

        return std::string("{}");
    }

    as::io_context io_context_;
    tcp::socket socket_;
};


AsioFuture::AsioFuture(AsioClient *client) : client{client} {};

std::string AsioFuture::get() 
{
    std::string result = client->_recv_result();
    return result;
};

inline bool AsioFuture::valid() 
{
    return true;
};