import os
import sys
import json
import time

# En primer lugar, importamos la variable que nos da la ruta a los paquetes instalados

install_path = os.getenv("INSTALL_PATH")
sys.path.insert(0, install_path)

print(install_path)

# importamos nuestro QClient, que nos va a permitir conectarnos a las QPUs
from python.qclient import QClient

# accedo al archivo json con información sobre las QPUs levantadas
STORE = os.getenv("STORE")
conf_file = STORE + "/.api_simulator/qpu.json"

client = QClient(conf_file)

with open(conf_file, 'r', encoding='utf-8') as archivo:
    datos = json.load(archivo)

# hago una lista de las rutas de las QPUs

QPUs = []

if isinstance(datos, dict):
    for k in datos.keys():
        client  = QClient(conf_file)
        client.connect(k)
        QPUs.append(client)
        

print("QPUs disponibles:")
for q in QPUs:
    print(q)


gg
    
# definimos un circuito en foramto string

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

results = []

for q in QPUs:
    results.append(q.send_circuit(circuit))

print("Se han envíado los", len(QPUs),"circuitos.")

print("Los resultados son:")

for k,r in zip(QPUs, results):
    print("QPU:", k,"-->\n",r.get())