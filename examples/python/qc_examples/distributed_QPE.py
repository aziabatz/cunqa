import os, sys
import numpy as np

# path to access c++ files
sys.path.append(os.getenv("HOME"))

from cunqa.qutils import get_QPUs, qraise, qdrop
from cunqa.circuit import CunqaCircuit
from cunqa.mappers import run_distributed
from cunqa.qjob import gather

def get_rz_circuits(n_ancilla_qubits, phase_to_compute):
    ancilla_circuit = CunqaCircuit(n_ancilla_qubits, id = "ancilla_circuit")
    register_circuit = CunqaCircuit(1, id = "register_circuit")

    register_circuit.x(0) # Rz statevector

    for i in range(n_ancilla_qubits):
        ancilla_circuit.h(i)

    for i in range(n_ancilla_qubits):
        with ancilla_circuit.expose(n_ancilla_qubits - 1 - i, register_circuit) as rcontrol:
            param = (2**i) * (2 * np.pi) * phase_to_compute
            register_circuit.crz(param, rcontrol, 0)

    
    if (n_ancilla_qubits % 2) == 0:
        swap_range = int(n_ancilla_qubits / 2)
    else:
        swap_range = int((n_ancilla_qubits - 1) / 2)

    for i in range(swap_range):
        ancilla_circuit.swap(i, n_ancilla_qubits - 1 - i)


    for i in range(n_ancilla_qubits):
        for j in range(i):
            angle  = (-np.pi) / (2**(i - j)) 
            ancilla_circuit.crz(angle, n_ancilla_qubits - 1 - j, n_ancilla_qubits - 1 - i)
        ancilla_circuit.h(n_ancilla_qubits - 1 - i)


    ancilla_circuit.measure_all()
    #register_circuit.measure_all()

    circuits_list = [ancilla_circuit, register_circuit]

    return circuits_list




if __name__ == "__main__":
    n_ancilla_qubits = 10
    n_register_qubits = 1
    phase_to_compute = 1/2**2

    QPE_circuits = get_rz_circuits(n_ancilla_qubits, phase_to_compute)

    #raised_qpus = qraise(2, "00:30:00", simulator="Aer", quantum_comm=True, cloud = True, family_name="for_dQPE")
    qpus  = get_QPUs(local=False, family = "for_dQPE")

    jobs = run_distributed(QPE_circuits, qpus, shots=100) 

    results = gather(jobs)

    print(results[0])