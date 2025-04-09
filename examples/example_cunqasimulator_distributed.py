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
        "qpus":["0", "1"]
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
        "qpus":["0", "1"]
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
        "qpus":["tcp://10.5.7.3:18858", "tcp://10.5.7.3:18857"]
    },
    {
        "name":"d_c_if_x",
        "qubits":[0,0,-1],
        "qpus":["tcp://10.5.7.3:18856", "tcp://10.5.7.3:18858"]
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
        "qpus":["tcp://10.5.7.3:18858", "tcp://10.5.7.3:18857"]
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
        "qpus":["tcp://10.5.7.3:18856", "tcp://10.5.7.3:18858"]
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
qpu2 = qpus[2]

print("QPUs disponibles:")
for q in qpus:
    print(f"QPU {q.id}, backend: {q.backend.name}.")

#TODO: Manage the number of shots authomatically. They have to be the same for all jobs
job0 = qpu0.run(d_qc_0_zmq, shots=1000) 
job1 = qpu1.run(d_qc_1_zmq, shots=1000)
job2 = qpu2.run(d_qc_2_zmq, shots=1000)
print("Result QPU0", job0.result().get_counts())
print("Result QPU1", job1.result().get_counts())
print("Result QPU2", job2.result().get_counts())









