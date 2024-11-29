#include <pybind11/pybind11.h>
#include "comm/client.hpp"
 
namespace py = pybind11;
 
PYBIND11_MODULE(c_client_class, m) {
 
    m.doc() = "Modulo de la clase Client() en c++";
 
    py::class_<Client>(m, "Client")
 
        .def(py::init<>())  // Constructor sin argumentos
 
        // .def(py::init<const std::string &, int>(), py::arg())  // Constructor
 
        .def("connect", &Client::connect)        // Metodo
 
        .def("read_result", &Client::read_result) // Metodo
 
        .def("send_data", py::overload_cast<const std::string&>(&Client::send_data)) 
        .def("send_data", py::overload_cast<std::ifstream&>(&Client::send_data)) 

        .def("stop", &Client::stop); // Metodo
 
}
 
int main(){
    return 0;
}


/*

module load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 scalapack/2.2.0 openblas/system
 
AER_INCL_DIR=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation/include/aer-cpp/src
PYBIND_INCL_DIR=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/conda/envs/qiskit-aer/include/pybind11
PYTHON_INCL_DIR=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/conda/envs/qiskit-aer/include/python3.9
PYTHON_LIB=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/conda/envs/qiskit-aer/lib
API_INCLUDES=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation/include
CUSTOM_JSON_LIB=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation/lib
 
 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PYTHON_LIB
 
 
g++ -std=c++17 pybind_test.cpp -o main.out -I$AER_INCL_DIR -I$PYBIND_INCL_DIR -I$PYTHON_INCL_DIR -I$API_INCLUDES -L$CUSTOM_JSON_LIB -L$PYTHON_LIB -fopenmp -llapack -lblas -fPIC -lpython3.9 -lmpi -lcustom-json
 

*/
