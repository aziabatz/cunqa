import os, sys

# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)


from qiskit import QuantumCircuit
from qiskit.circuit.library import QFT
from cunqa import getQPUs

qpus  = getQPUs(local=False)

for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}, simulator: {q.backend.simulator}, version: {q.backend.version}.")

n = 5 # number of qubits
qc = QuantumCircuit(n)
qc.x(0); qc.x(n-1); qc.x(n-2)
qc.append(QFT(n), range(n))
qc.append(QFT(n).inverse(), range(n))
qc.measure_all()

counts = []

for i, qpu in enumerate(qpus):

    print(f"For QPU {qpu.id}, with backend {qpu.backend.name}:")
    qjob = qpu.run(qc, transpile = True, shots = 1000)# non-blocking call
    result = qjob.result() # bloking call
    time = qjob.time_taken()
    counts.append(result.get_counts())

    print(f"Result: \n{result.get_counts()}\n Time taken: {time} s.")
