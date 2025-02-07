import os, sys

# adding pyhton folder path to detect modules
#sys.path.insert(0, "/mnt/netapp1/Store_CESGA/home/cesga/mlosada/api/api-simulator/cunqa")

# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

# Let's get the QPUs raised

from cunqa.qpu import getQPUs

qpus  = getQPUs()

for q in qpus:
    print(f"QPU {q.id}, name: {q.backend.name}, backend: {q.backend.simulator}, version: {q.backend.version}.")


# Let's create a circuit to run in our QPUs!

from qiskit import QuantumCircuit
from qiskit.circuit.library import QFT

n = 5 # number of qubits

qc = QuantumCircuit(n)

qc.x(0); qc.x(n-1); qc.x(n-2)

qc.append(QFT(n), range(n))

qc.append(QFT(n).inverse(), range(n))

qc.measure_all()


counts = []

for i, qpu in enumerate([qpus[0], qpus[2], qpus[4], qpus[5]]):

    print(f"For QPU {qpu.id}, with backend {qpu.backend.name}:")

    if i == 3:
        qjob = qpu.run(qc, transpile = True, initial_layout = [31, 30, 29, 28, 27], shots = 1000)
    else:
        qjob = qpu.run(qc, transpile = True, shots = 1000)

    result = qjob.result() # bloking call

    time = qjob.time_taken()

    counts.append(result.get_counts())

    print(f"Result: \n{result.get_counts()}\n Time taken: {time} s.")


from qiskit.visualization import plot_histogram
import matplotlib.pyplot as plt
plot_histogram(counts, figsize = (10, 5), bar_labels=False); plt.legend(["QPU 0", "QPU 2", "QPU 4", "QPU 5"])
plt.savefig('counts.png')
plt.show()

# Cool isn't it? But this circuit is too simple, let's try with a more complex one!

import json

with open("circuits/circuit_15qubits_10layers.json", "r") as file:
    circuit = json.load(file)

# This circuit has 15 qubits and 10 intermidiate measurements, let's run it in AerSimulator

for qpu in qpus:
    if qpu.backend.name == "BasicAer":
        qpu0 = qpu
        break

qjob = qpu0.run(circuit, transpile = True, shots = 1000)

result = qjob.result() # bloking call

time = qjob.time_taken()

counts.append(result.get_counts())

print(f"Result: \n{result.get_counts()}\n Time taken: {time} s.")

# Takes much longer ... let's parallelize 3 executions in 3 different QPUs

# Remenber that sending circuits to a given QPU is a non blocking call, so we can use a loop, keeping the QJOb objects in a list:

qjobs = []

for qpu in [qpus[0], qpus[2], qpus[4]]:
    qjobs.append(qpu.run(circuit, transpile = True, shots = 1000))

# Then, we can wait for all the jobs to finish with the gather function. Let's measure time to check that we are parallelizing:
import time
from cunqa.qpu import gather

tick = time.time()
results = gather(qjobs) # this is a bloking call
tack = time.time()

print(f"Time taken to run 3 circuits in parallel: {tack - tick} s.")
print("Time for each execution:")
for i, result in enumerate(results):
    print(f"For job {i}, time taken: {result.time_taken} s.")

