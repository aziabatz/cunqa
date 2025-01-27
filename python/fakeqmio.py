import os
import argparse
import json



from qmiotools.integrations.qiskitqmio import FakeQmio 
from qiskit_aer.noise import NoiseModel

STORE_PATH = os.getenv("STORE")
INSTALL_PATH = os.getenv("INSTALL_PATH")


parser = argparse.ArgumentParser(description="FakeQmio from calibrations")

parser.add_argument("backend_path", type = str, help = "Path to backend config json")

args = parser.parse_args()

if (args.backend_path == "last_calibrations"):
    fakeqmio = FakeQmio()
else:
    fakeqmio = FakeQmio(calibration_file = args.backend_path)

noise_model = NoiseModel.from_backend(fakeqmio)
noise_model_json = noise_model.to_dict(serializable = True)

with open(INSTALL_PATH + "/include/utils/basis_gates.json", "r") as gates_file:
    gates = json.load(gates_file)

backend_json = {
    "backend":{
        "name": "FakeQmio", 
        "version": "",
        "simulator": "AerSimulator",
        "n_qubits": 32, 
        "url": "",
        "is_simulator": True,
        "conditional": True, 
        "memory": True,
        "max_shots": 1000000,
        "description": "FakeQmio backend",
        "basis_gates": gates["fakeqmio"], 
        "custom_instructions": "",
        "gates": []
    },
    "noise":noise_model_json
}


with open("{}/.api_simulator/fakeqmio_backend.json".format(STORE_PATH), 'w') as file:
    json.dump(backend_json, file)
