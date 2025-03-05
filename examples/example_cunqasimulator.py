import os
import sys
import json
import time

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)
from cunqa.qclient import QClient

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
        "name": "h",
        "qubits": [0,-1,-1]
    },
    {
        "name": "cx",
        "qubits": [0, 1,-1]
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


print("Cliente: " + claves_primer_nivel[0])

future1 = client.send_circuit(circuit)

print("Future creado.")

print("RESULT:" + future1.get()) 
