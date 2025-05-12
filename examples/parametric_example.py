import os
import sys
import time
#import scipy.optimize
import random

from qiskit import QuantumCircuit

# adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)


# loading necesary modules

from cunqa.qutils import getQPUs
qpus = getQPUs()


print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")

qpu0 = qpus[-1]

theta=1.57079
qc = QuantumCircuit(2)
qc.rx(theta, 0)
qc.measure_all()



job = qpu0.run(qc, transpile = True, shots=1000)
print(job.result().get_counts())


for _ in range(10):
    new_params = [random.uniform(0.0, 6.283185)]
    print("Parameter: ", new_params[0])
    param_job = job.upgrade_parameters(new_params)
    print("Counts: ", param_job.result().get_counts())
