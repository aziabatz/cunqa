import numpy as np

# Import Qiskit
from qiskit import QuantumCircuit, transpile
from qiskit_aer import AerSimulator
from qiskit_aer.backends.aer_compiler import assemble_circuits
from qiskit.visualization import plot_histogram, plot_state_city
import qiskit.quantum_info as qi
from qmiotools.integrations.qiskitqmio import FakeQmio

import json

simulator = FakeQmio()
#print(simulator.configuration.__str__())

# Create circuit
circ = QuantumCircuit(2)
circ.h(0)
circ.cx(0, 1)
circ.measure_all()

print

circ = transpile(circ, simulator)

print(circ.layout)

"""
print(circ.num_qubits)

circuit, _ = assemble_circuits([circ])

print(circuit) """

"""# Transpile for simulator
simulator = AerSimulator()
circ = transpile(circ, simulator)

# Run and get counts
result = simulator.run(circ, shots=10, memory=True).result()
memory = result.get_memory(circ)
print(memory)

from qiskit import assemble


qc = QuantumCircuit(2, name='Bell', metadata={'test': True})
qc.h(0)
qc.cx(0, 1)
qc.measure_all()

json_str = json.dumps(assemble(qc).to_dict())

print(json_str)

with open('circuit.json', 'w', encoding='utf-8') as f:
    json.dump(assemble(qc).to_dict(), f, ensure_ascii=False, indent=4) """

