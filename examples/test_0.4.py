import os
import sys
import json
import time

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
sys.path.insert(0, "/mnt/netapp1/Store_CESGA/home/cesga/mlosada/api/api-simulator/python")

from qpu import QPU, getQPUs
from qjob import gather
from qiskit import QuantumCircuit
# from scipy.optimize import differential_evolution


lista = getQPUs()

print(f"QPUs we are going to work with ({len(lista)}): ")
print(" ")
for q in lista:
    print(f"QPU {q.id_}: {q.backend.name}")


with open("circuit_15qubits_10layers.json", "r") as file:
    circuit = json.load(file)


qjobs = []

tick = time.time()
for q in lista[:4]:
    qjobs.append(q.run(circ = circuit, shots=222))
    print("ya")

for qj in qjobs:
    print(qj.state())

print("Dentro gather: ")

resultados = gather(qjobs)

tack = time.time()

print("Terminó en :", tack-tick, " s")

for i,r in enumerate(resultados):
    print(f"For QPU {i}: {r.get_counts()}")




