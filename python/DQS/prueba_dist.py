from qiskit import QuantumCircuit
from backend import DistQPU

qd = DistQPU()

back = qd.set_backend()
list_backends = [back, back, back]

qc = QuantumCircuit(2, 2)
qc.h(0)
qc.cx(0, 1)
qc.measure([0, 1], [0, 1])

list_circs = [qc, qc, qc]

list_shots = [100,50,200]

results = qd.run(list_circs=list_circs, list_backends=list_backends, list_shots=list_shots)

print(results)

