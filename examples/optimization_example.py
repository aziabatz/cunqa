from scipy.optimize import differential_evolution
import pandas as pd
from qiskit import QuantumCircuit
from qiskit.circuit import Parameter
import numpy as np
import os, sys
# adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)

# primero genero un circuito, vamos a escoger un hardware efficient ansatz
def hardware_efficient_ansatz(num_qubits, num_layers):
    qc = QuantumCircuit(num_qubits)
    param_idx = 0
    for _ in range(num_layers):
        for qubit in range(num_qubits):
            phi = Parameter(f'phi_{param_idx}_{qubit}')
            lam = Parameter(f'lam_{param_idx}_{qubit}')
            qc.ry(phi, qubit)
            qc.rz(lam, qubit)
        param_idx += 1
        for qubit in range(num_qubits - 1):
            qc.cx(qubit, qubit + 1)

    qc.measure_all()
    return qc

num_qubits = 6
num_layers = 3
ansatz = hardware_efficient_ansatz(num_qubits, num_layers)
num_parameters = ansatz.num_parameters
print(num_parameters)

################################################################################################

from cunqa import getQPUs, QJobMapper, gather
from qiskit.circuit import Parameter

qpus = getQPUs()

for qpu in qpus:
    print(f"QPU ID: {qpu.id}")


def opa(state, groups):
    energy = 0
    for i in groups:
        if state[i] == 0:
            energy += 1
        else:
            energy -= 1
    return energy


import time


########################### DENTRO OPTIMIZACIÓN SECUENCIAL ########################################

print("Dentro secuencial")
print()
print()
tick = time.time()
init_qjobs = []
init_params = np.zeros(num_parameters)
for q in [qpus[0]]:
    init_qjobs.append(q.run(ansatz.assign_parameters(init_params), transpile=False, shots=1024))


def cost_function(result):
    GROUPS = list(range(num_qubits))  # grupos de qubits
    n_shots = 1024

    # send to QPU and call result
    counts = result.get_counts()
    
    pdf = pd.DataFrame.from_dict(counts, orient="index").reset_index()
    pdf.rename(columns={"index": "state", 0: "counts"}, inplace=True)
    pdf["probability"] = pdf["counts"] / n_shots
    # pdf["energy"] = pdf.apply(lambda x: x, axis=1)
    return sum(opa(state, GROUPS) for state in pdf["state"])/n_shots


mapper = QJobMapper(init_qjobs)

pop=[]
total_pop=1*num_parameters
for j in range(total_pop):
    initial_point=np.random.uniform(-np.pi, np.pi, num_parameters)
    pop.append(initial_point)

bounds=[]
for i in range(0,num_parameters):
    bounds.append((-np.pi,np.pi))

print("Bounds:", len(bounds))
print("Initial population:", len(pop))

best_individual = []

def cb(xk,convergence=1e-8):
     best_individual.append(xk)

result = differential_evolution(cost_function, bounds, maxiter=500, disp=True, workers=mapper, strategy='best1bin', init=pop, polish = False, callback=cb)

print(result)

energies = mapper(cost_function, best_individual)




tack = time.time()
print("Time:", tack-tick)

kk

########################### DENTRO OPTIMIZACIÓN PARALELA ########################################

print("Dentro paralela")
print()
print()
tick = time.time()


init_qjobs = []
init_params = np.ones(num_parameters)*np.pi
for q in qpus:
    init_qjobs.append(q.run(ansatz.assign_parameters(init_params), transpile=False, shots=1024))

from cunqa.logger import logger

logger.debug("Dentro gather")
results = gather(init_qjobs)
logger.debug("Gather finished")

def cost_function(result):
    GROUPS = list(range(num_qubits))  # grupos de qubits
    n_shots = 1024

    # send to QPU and call result
    counts = result.get_counts()
    
    pdf = pd.DataFrame.from_dict(counts, orient="index").reset_index()
    pdf.rename(columns={"index": "state", 0: "counts"}, inplace=True)
    pdf["probability"] = pdf["counts"] / n_shots
    # pdf["energy"] = pdf.apply(lambda x: x, axis=1)
    return sum(opa(state, GROUPS) for state in pdf["state"])/n_shots

init_params = np.zeros(num_parameters)

mapper = QJobMapper(init_qjobs)



# vamos a probar una iteración
pop=[]
total_pop=1*num_parameters
for j in range(total_pop):
    initial_point=np.random.uniform(-np.pi, np.pi, num_parameters)
    pop.append(initial_point)

bounds=[]
for i in range(0,num_parameters):
    bounds.append((-np.pi,np.pi))

print("Bounds:", len(bounds))
print("Initial population:", len(pop))

result = differential_evolution(cost_function, bounds, maxiter=500, disp=True, seed=188, workers=mapper, strategy='best1bin', init=pop, polish = False)

print(result)

tack = time.time()
print("Time:", tack-tick)




