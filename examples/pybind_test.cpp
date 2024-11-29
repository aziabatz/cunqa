#include <pybind11/pybind11.h>
#include "comm/client.hpp"

namespace py = pybind11;

PYBIND11_MODULE(c_client_class, m) {

	m.doc() = "Modulo de la clase Client() en c++";

    py::class_<Client> client(m, "Client");

    client.def(py::init<>());  // Constructor sin argumentos

		// .def(py::init<const std::string &, int>(), py::arg())  // Constructor

    client.def("connect", &Client::connect);        // Metodo

    client.def("read_result", &Client::read_result); // Metodo

	client.def("send_data", py::overload_cast<const std::string&>(&Client::send_data));
    client.def("send_data", py::overload_cast<std::ifstream&>(&Client::send_data));

	client.def("stop", &Client::stop); // Metodo

}
 
int main(){
    return 0;
}