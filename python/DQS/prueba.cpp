
#include <iostream>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// Function to sum two numbers
int sum(int a, int b) {
    return a + b;
}

PYBIND11_MODULE(prueba, m) {
	m.def("sum", &sum, "A function that adds two numbers");
}
