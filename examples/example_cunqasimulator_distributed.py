import os
import sys
import json
import time

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
from cunqa.qpu import getQPUs


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
    },
    "exec_type":"dynamic"
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
    },
    "exec_type":"dynamic"
}

d_qc_0_zmq = {
    "instructions": [

    {
        "name":"h",
        "qubits":[0,-1,-1],
    },
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":["tcp://10.5.7.13:17715", "tcp://10.5.7.13:17714"]
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
        "qubits":[0,0,-1],
        "qpus":["tcp://10.5.7.13:17715", "tcp://10.5.7.13:17714"]
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


qpus = getQPUs()
qpu0 = qpus[0]
qpu1 = qpus[1]

print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")


job0 = qpu0.run(d_qc_0_zmq, shots=10)
job1 = qpu1.run(d_qc_1_zmq, shots=10)
print(job0.result().get_counts())
print(job1.result().get_counts())









