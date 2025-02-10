import os
import sys
import time

from qiskit import QuantumCircuit

# adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)


# loading necesary modules

from cunqa.qpu import getQPUs, gather
qpus = getQPUs()


print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")

qpu0 = qpus[-1]


theta=0.01

qc = QuantumCircuit(2,2)
qc.rx(theta, 0)
qc.h(0)
qc.measure(0,0)
qc.h(1)
qc.measure_all()


job = qpu0.run(qc, transpile = True, shots = 10)
print(job)
print(job.result().get_counts())



new_params = [theta]
for i in range(10):
    new_params = [new_params[0]/(i+1)]
    param_job = job.upgrade_parameters(new_params)
    print(param_job.result().get_counts())
