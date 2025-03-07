import os
import sys
import json
import time

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
from cunqa.qclient import QClient





STORE = os.getenv("STORE")
conf_file = STORE + "/.api_simulator/qpus.json"

with open(conf_file, 'r', encoding='utf-8') as archivo:
    datos = json.load(archivo)

if isinstance(datos, dict):
    claves_primer_nivel = list(datos.keys())

#for clave in claves_primer_nivel:
client = QClient(conf_file)
print("Cliente instanciado")

print(claves_primer_nivel)

client.connect(claves_primer_nivel[0])




# Ejemplo comunicaciones clasicas

qc = {
    "instructions": [
    

    {
        "name":"h",
        "qubits":[0,-1,-1]
    },
    {
        "name": "measure",
        "qubits": [0,-1,-1]
    }
    ],
    "num_qubits": 2,
    "num_clbits": 2,
    "classical_registers": {
        "meas": [
            0,
            1
        ]
    }
}

d_qc_0 = {
    "instructions": [
    

    {
        "name":"h",
        "qubits":[0,0,-1],
    },
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":[0,1]
    },
    {
        "name": "measure",
        "qubits": [0,-1,-1]
    },
    {
        "name": "measure",
        "qubits": [1,-1,-1]
    }
    ],
    "num_qubits": 2,
    "num_clbits": 2,
    "classical_registers": {
        "meas": [
            0,
            1
        ]
    }
}

d_qc_1 = {
    "instructions": [
    
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":[0,1]
    },
    {
        "name": "measure",
        "qubits": [0,-1,-1]
    },
    {
        "name": "measure",
        "qubits": [1,-1,-1]
    }
    ],
    "num_qubits": 2,
    "num_clbits": 2,
    "classical_registers": {
        "meas": [
            0,
            1
        ]
    }
}


distributed_circuit = """
{
    "config": {
        "shots": 19,
        "method": "automatic",
        "memory_slots": 7,
        "seed": 1331
    },
    "instructions": [
    {
        "qpu_id": 0,
        "circuit": {d_qc_0}
    },
    {
        "qpu_id": 1,
        "circuit": {d_qc_1}
    }
    ]
}
""" 

from cunqa.qpu import getQPUs
qpus = getQPUs()
qpu0 = qpus[0]
qpu1 = qpus[1]

print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")


job0 = qpu0.run(d_qc_0, shots=19)
job1 = qpu1.run(d_qc_1, shots=19)
print(job0.result().get_counts())
print(job1.result().get_counts())




#Ejemplo funcional basico

circuit = """
{
    "config": {
        "shots": 19,
        "method": "automatic",
        "memory_slots": 7,
        "seed": 1331
    },
    "instructions": [
    
    {
        "name":"rx",
        "qubits":[0,-1,-1],
        "params": [1.1235]
    },
    {
        "name": "measure",
        "qubits": [0,-1,-1]
    },
    {
        "name": "measure",
        "qubits": [1,-1,-1]
    }
    ]
}
""" 

new_param = """{
    "params":[0.0]
    }"""

""" 
future1 = client.send_circuit(circuit)

print("Future creado.")

print("RESULT:" + future1.get()) 


future2 = client.send_parameters(new_param)

print("Future 2 creado.")

print("RESULT_PARAMS:" + future2.get())  """





#Ejemplo con la API en Python (de momento no funciona)
""" import os
import sys
import time
#import scipy.optimize
import random

from qiskit import QuantumCircuit

# adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)


# loading necesary modules

from cunqa.qpu import getQPUs
qpus = getQPUs()


print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")

qpu0 = qpus[-1]

theta=1.57079
qc = QuantumCircuit(2)
qc.rx(theta, 0)
qc.measure_all()



job = qpu0.run(qc, transpile = True, shots=19)
print(job.result().get_counts())


for _ in range(10):
    new_params = [random.uniform(0.0, 6.283185)]
    print("Parameter: ", new_params[0])
    param_job = job.upgrade_parameters(new_params)
    print("Counts: ", param_job.result().get_counts())
 """