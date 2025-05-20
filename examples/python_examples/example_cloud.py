import os, sys

# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

from cunqa import getQPUs
from cunqa.circuit import CunqaCircuit

qpus  = getQPUs(local=False)

for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}, simulator: {q.backend.simulator}, version: {q.backend.version}.")

qc = CunqaCircuit(2, 2)
qc.h(0)
qc.cx(0, 1)
qc.measure_all()

qpu = qpus[0]
qjob = qpu.run(qc, transpile=True, shots = 1000)# non-blocking call

counts = qjob.result.counts
time = qjob.time_taken

print(f"Result: \n{counts}\n Time taken: {time} s.")
