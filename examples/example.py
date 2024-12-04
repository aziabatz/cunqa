import os
import sys

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)

from python.client import Client

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

client = Client("/mnt/netapp1/Store_CESGA//home/cesga/jvazquez/.api_simulator/qpu.json")

client.connect(1)
client.send_data(circuit)
result = client.read_result()
client.send_data("CLOSE")

print(result)