import os
import sys
import json
from qiskit import QuantumCircuit

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
print(sys.path)
from python.client import Client
from python.cluster import QPU, from_qc_to_json
#from python import backend_options

json_path = "/mnt/netapp1/Store_CESGA/home/cesga/acarballido/.api_simulator"
with open("{}/qpu.json".format(json_path)) as net_json:
    qpus = json.load(net_json)
    aux_server_endpoint = "tcp://" + qpus["{}".format(0)]["IPs"]["bond0.117"] + ":" + str(qpus["{}".format(0)]["port"])




qpu = QPU(id_=0, server_endpoint = aux_server_endpoint)

qc = QuantumCircuit(2)
qc.h(0)
qc.cx(0,1)
qc.measure_all()

result = qpu.c_run(qc, shots=199)




circuit = """
{
    "config": {
        "shots": 1024,
        "method": "statevector",
        "memory_slots": 7
    },
    "instructions": [
    {
        "name": "h",
        "qubits": [0]
    },
    {
        "name": "cx",
        "qubits": [0, 1]
    },
    {
        "name": "measure",
        "qubits": [0],
        "memory": [0]
    },
    {
        "name": "measure",
        "qubits": [1],
        "memory": [1]
    }
    ]
}
"""

#client = Client("/mnt/netapp1/Store_CESGA//home/cesga/acarballido/.api_simulator/qpu.json")

#client.connect(2)
#client.send_data(circuit)
#result = client.read_result()
#client.send_data("CLOSE")

print(result)
