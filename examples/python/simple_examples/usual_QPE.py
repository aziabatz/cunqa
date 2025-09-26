import os, sys
import numpy as np

# path to access c++ files
sys.path.append(os.getenv("HOME"))

from cunqa.qutils import get_QPUs, qraise, qdrop
from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator


def get_rz_circuit_for_QPE(n_ancilla_qubits, phase_to_compute):
    circuit = QuantumCircuit(n_ancilla_qubits + 1, n_ancilla_qubits)

    circuit.x(n_ancilla_qubits)
    for i in range(n_ancilla_qubits):
        circuit.h(i)
    for i in range(n_ancilla_qubits):
        param = (2**i) * (2*np.pi) * phase_to_compute
        circuit.crz(param, n_ancilla_qubits - 1 - i, n_ancilla_qubits)
    
    if (n_ancilla_qubits % 2) == 0:
        swap_range = int(n_ancilla_qubits / 2)
    else:
        swap_range = int((n_ancilla_qubits - 1) / 2)

    for i in range(swap_range):
        circuit.swap(i, n_ancilla_qubits - 1 - i)


    for i in range(n_ancilla_qubits):
        for j in range(i):
            angle  = (-np.pi) / (2**(i - j)) 
            circuit.crz(angle, n_ancilla_qubits - 1 - j, n_ancilla_qubits - 1 - i)
        circuit.h(n_ancilla_qubits - 1 - i)

    for i in range(n_ancilla_qubits):
        circuit.measure(i, i)

    return circuit


if __name__ == "__main__":
    n_ancilla_qubits = 10
    phase_to_compute = 1/2**5
    circuit = get_rz_circuit_for_QPE(n_ancilla_qubits, phase_to_compute)

    print(circuit)

    simulator = AerSimulator()

    result = simulator.run(circuit, shots = 100, method = "statevector").result()

    print(result.data()["counts"])
