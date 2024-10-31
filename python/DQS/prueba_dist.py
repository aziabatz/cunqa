from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator
#from backend import DistQPU, DistQPU2, QPU
#from backend import get_qpus, set_backend, set_mult_backends
import backend

qpus = backend.get_qpus()

#back = "backend_personalizado"
#list_backs = [back, back]

#backend.set_mult_backends(list_backs, qpus)

#for i in range(len(qpus)):
#	print(qpus[i].backend)

#qpu1 = QPU()
#qpu1.assign_qpu("0")

#qpu2 = QPU()
#qpu2.assign_qpu("1")
#back = AerSimulator()

#list_backends = [back, back]


qc = QuantumCircuit(2, 2)
qc.h(0)
qc.cx(0, 1)
qc.measure([0, 1], [0, 1])



#result = backend.run(circ=qc, qpu=qpus[0], shots = 100).result()
#result_2 = backend.run(qc, list_qpus=qpus, id="1", shots=200).result()
result_3 = backend.run(qc, id="1", shots=150).result()

print(result_3)




#list_circs = [qc, qc]
#list_qpus = [qpu1, qpu2]

#shots = [100,50]

#results = dqpu.run(list_circs=list_circs, list_shots=list_shots)

#print(results[0].result())

#dqpu = DistQPU2()
#dqpu.set_qpus(list_qpus)

#print(dqpu.list_qpus)

#result = dqpu.dist_run(list_circs, list_shots=shots)

#aux = result[1].result()

#print(aux)

#print("El ID de la QPU es: ", qpu.server_id)
#print("Las caracteristicas del server son: ", qpu.server)

#res = qpu.run(qc, shots=10)
#res = res.result()
#print(res)
