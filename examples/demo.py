import os, sys

# adding pyhton folder path to detect modules
sys.path.insert(0, "/mnt/netapp1/Store_CESGA/home/cesga/mlosada/api/api-simulator/cunqa")

# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

# Let's get the QPUs raised

from qpu import getQPUs

qpus  = getQPUs()

for q in qpus:
    print(f"QPU {q.id}, name: {q.backend.name}, backend: {q.backend.simulator}, version: {q.backend.version}.")


# Let's create a circuit to run in our QPUs!

from qiskit import QuantumCircuit
from qiskit.circuit.library import QFT

n = 10 # number of qubits

qc = QuantumCircuit(n)

qc.x(0); qc.x(n-1); qc.x(n-2)

qc.append(QFT(n), range(n))

qc.append(QFT(n).inverse(), range(n))

qc.measure_all()


# Let's run the circuit in the first QPU

qpu = qpus[0]

counts = []
for qpu in [qpus[0], qpus[2], qpus[4]]:

    print(f"For QPU {qpu.id}, with backend {qpu.backend.name}:")

    qjob = qpu.run(qc, transpile = True, shots = 1000)

    result = qjob.result() # bloking call

    time = qjob.time_taken()

    counts.append(result.get_counts())

    print(f"Result: \n{result.get_counts()}\n Time taken: {time} s.")

from qiskit.visualization import plot_histogram
import matplotlib.pyplot as plt
plot_histogram(counts)
plt.savefig('histogram.png')
plt.show()
