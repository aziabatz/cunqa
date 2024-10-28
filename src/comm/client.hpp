#include <iostream>
#include <fstream>
#include <memory>
#include "../utils/ip_config.hpp"

#if COMM_LIB == ASIO
    #include "strategies/asio/asio_client.hpp"
    using SelectedClient = AsioClient;
#elif COMM_LIB == ZMQ
    #include "strategies/zmq/zmq_comm.hpp"
    using SelectedClient = ZMQClient;
#elif COMM_LIB == CROW
    #include "strategies/crow/crow_comm.hpp"
    using SelectedClient = CrowClient;
#else
    #error "A valid library should be defined (ASIO, ZMQ o CROW) in COMM_LIB."
#endif

class Client {
    std::unique_ptr<SelectedClient> strategy;
public:

    Client() :
    strategy{std::make_unique<SelectedClient>()} { }

    void connect() {
        
        // TODO: Move the reading of the file to another place
        
        std::ifstream file("qpu.json");
        if (!file.is_open()) {
            std::cerr << "Cannot open the JSON file\n";
        }

        // Parsear el JSON
        nlohmann::json jsonData;
        try {
            file >> jsonData;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Error parsing the JSON: " << e.what() << "\n";
        }

        std::string hostname = jsonData["0"][0]["hostname"];
        std::string port = jsonData["0"][0]["port"];

        IPConfig server_ip_config{hostname, port};
        strategy->connect(server_ip_config);
    }

    void write(const std::string& data) {
        strategy->write(data);
    }

    void read(){
        strategy->read();
    }

    void stop(){
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
