#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace std::string_literals;
using boost::asio::ip::tcp;

class AsioServer {
    IPConfig ip_config;
    boost::asio::io_context io_context;
public:
    AsioServer(const IPConfig& ip_config) :
    acceptor_(io_context, tcp::endpoint(tcp::v4(), std::stoi(ip_config.port))),
    ip_config{ip_config} {  }

    void start(){
        startAccept();
        io_context.run();
    }

    void stop(){
        io_context.stop();
    }

private:
    void startAccept() {
        auto socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
            if (!error) {
                std::cout << "Client connected: " << socket->remote_endpoint() << std::endl;
                receiveQASM(socket);
            } else {
                std::cerr << "Accept error: " << error.message() << std::endl;
            }
            startAccept();
        });
    }

    void receiveQASM(const std::shared_ptr<tcp::socket>& socket) {
        auto buffer = std::make_shared<std::string>();

        boost::asio::async_read_until(*socket, boost::asio::dynamic_buffer(*buffer), '\n',
            [this, socket, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    std::string qasm = buffer->substr(0, bytes_transferred);

                    std::cout << "Mensaje recibido: " << qasm << std::endl;
                    std::string result = simulateQuantumCircuit(qasm);
                    
                    std::cout << result;
                    
                    sendResult(socket, result);
                } else {
                    std::cerr << "Read error: " << error.message() << std::endl;
                }
            }
        );
    }

    void sendResult(const std::shared_ptr<tcp::socket>& socket, const std::string& result) {
        auto buffer = std::make_shared<std::string>(result);
        boost::asio::async_write(*socket, boost::asio::buffer(*buffer),
            [buffer](const boost::system::error_code& error, std::size_t) {
                if (!error) {
                    std::cout << "Result sent to client." << std::endl;
                } else {
                    std::cerr << "Write error: " << error.message() << std::endl;
                }
            }
        );
    }

    std::string simulateQuantumCircuit(const std::string& qasm) {

        std::cout << "Circuito simulado\n";
        return "Circuito simulado\n"s;

        /* // Llamada al simulador de Qiskit Aer (Python) o cualquier otro simulador
        // Esto puede hacerse escribiendo el QASM a un archivo temporal y ejecutando un script Python
        
        std::string command = "python simulate.py \"" + qasm + "\"";
        std::string result;
        char buffer[128];

        // Abrir un pipe para leer la salida del comando
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return "Simulation failed!";
        }

        // Leer la salida del pipe
        while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
            result += buffer;
        }

        // Cerrar el pipe
        pclose(pipe);
        
        return result; */
    }

    tcp::acceptor acceptor_;
};

