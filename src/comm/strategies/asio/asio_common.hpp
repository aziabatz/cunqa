#pragma once

#include <boost/asio.hpp>
#include "../../../utils/helpers.hpp"
#include <iostream>

using boost::asio::ip::tcp;

void send_string(const std::shared_ptr<tcp::socket>& socket, const std::string& data)
{
    std::cout << "\n--- SEND ---\n";
    auto data_length = legacy_size_cast<uint32_t, std::size_t>(data.size());
    auto data_length_network = htonl(data_length);
    auto buffer = boost::asio::buffer(&data_length_network, sizeof(data_length_network));

    std::cout << "Tamano mensaje: " << data_length << "\n";
    // Send string size
    boost::asio::async_write(*socket, buffer,
        [](const boost::system::error_code& error, std::size_t) 
        { }
    );

    std::cout << "Mensaje: " << data << "\n\n";
    // Send actual string
    boost::asio::async_write(*socket, boost::asio::buffer(data),
        [](const boost::system::error_code& error, std::size_t) 
        { }
    );
}

std::string read_string(tcp::socket& socket)
{
    std::cout << "\n--- RECEIVE ---\n";
    // Receive string size
    uint32_t data_length_network;
    boost::asio::read(socket, boost::asio::buffer(&data_length_network, sizeof(data_length_network)));
    uint32_t data_length = ntohl(data_length_network);

    std::cout << "Tamano mensaje: " << data_length << "\n";

    // Send actual string
    std::string data(data_length, '\0');
    boost::asio::read(socket, boost::asio::buffer(&data[0], data_length));
    std::cout << "Mensaje: " << data << "\n\n";

    return data;
}