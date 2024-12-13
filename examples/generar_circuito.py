import os
import sys
from qiskit import QuantumCircuit
import numpy as np
import json

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
sys.path.insert(0, "/mnt/netapp1/Store_CESGA/home/cesga/mlosada/api/api-simulator/python")

from circuit import qc_to_json

num_qubits = 15

layers = 10

params = []

for l in range(layers):
    params.append(np.random.uniform(0, 2 * np.pi, num_qubits))


print(params)
                  
qc = QuantumCircuit(num_qubits,num_qubits)

qc.h(range(num_qubits))

for j,pa in enumerate(params):

    for i in range(num_qubits-1):
        if i%2 == 0 :
            qc.cx(i,i+1)
    
    for i,p in enumerate(pa):
        qc.rx(p, i)
        qc.ry(p,i)
    qc.measure(j,j)
    
    qc.h(range(num_qubits))

qc.measure_all()

print(qc)


qc_string = qc_to_json(qc)


with open("circuit_"+str(num_qubits)+"qubits_"+str(layers)+"layers.json", "w") as file:
    json.dump(qc_string, file)













