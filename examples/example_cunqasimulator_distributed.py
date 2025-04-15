import os
import sys
import json
import time

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
from cunqa.qutils import getQPUs

qpus = getQPUs(local=False)
print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")

qpu0 = qpus[0]
qpu1 = qpus[1]
qpu2 = qpus[2]

d_qc_0_zmq = {
    "instructions": [

    {
        "name":"h",
        "qubits":[0,-1,-1],
    },
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":["{}".format(qpu0.endpoint), "{}".format(qpu1.endpoint)]
    },
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":["{}".format(qpu2.endpoint), "{}".format(qpu0.endpoint)]
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
    },
    "exec_type":"dynamic"
}

d_qc_1_zmq = {
    "instructions": [
    {
        "name":"d_c_if_x",
        "qubits":[1,0,-1],
        "qpus":["{}".format(qpu0.endpoint), "{}".format(qpu1.endpoint)]
    },
    {
        "name":"h",
        "qubits":[0,-1,-1],
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
    },
    "exec_type":"dynamic"
}

d_qc_2_zmq = {
    "instructions": [
    {
        "name":"h",
        "qubits":[1,-1,-1],
    },
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":["{}".format(qpu2.endpoint), "{}".format(qpu0.endpoint)]
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
    },
    "exec_type":"dynamic"
}

print(d_qc_0_zmq)
#TODO: Manage the number of shots authomatically. They have to be the same for all jobs
job0 = qpu0.run(d_qc_0_zmq, shots=10) 
job1 = qpu1.run(d_qc_1_zmq, shots=10)
job2 = qpu2.run(d_qc_2_zmq, shots=10)
print("Result QPU0", job0.result().get_counts())
print("Result QPU1", job1.result().get_counts())
print("Result QPU2", job2.result().get_counts())









