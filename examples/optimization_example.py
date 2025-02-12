from scipy.optimize import differential_evolution, Bounds
import re
from scipy.optimize import OptimizeResult
from qiskit.quantum_info import SparsePauliOp
from scipy.optimize import minimize
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
    params = np.random.uniform(0, 2 * np.pi, size=(num_layers, num_qubits * 3))
    param_idx = 0
    for _ in range(num_layers):
        for qubit in range(num_qubits):
            theta = Parameter(f'theta_{param_idx}_{qubit}')
            phi = Parameter(f'phi_{param_idx}_{qubit}')
            lam = Parameter(f'lam_{param_idx}_{qubit}')
            qc.rx(theta, qubit)
            qc.ry(phi, qubit)
            qc.rz(lam, qubit)
        param_idx += 1
        for qubit in range(num_qubits - 1):
            qc.cx(qubit, qubit + 1)
    return qc

num_qubits = 10
num_layers = 5
ansatz = hardware_efficient_ansatz(num_qubits, num_layers)
num_parameters = ansatz.num_parameters

print(num_parameters)

################################################################################################


from cunqa.qpu import getQPUs, QPUMapper, QJobMapper
from qiskit.circuit import Parameter

qpus = getQPUs()

# mapper = QPUMapper(qpus) # esto va a ir a los workers

# ahora tengo que definir una función de coste que tomará un argumento que es una tupla de los parámetros y la qpu


def opa(state, group):
    # group: los qubits, voy a hacerlos para cada uno
    # Define the observable for the cost function
    observable = SparsePauliOp.from_list([("Z" * len(group), 1)])
    return observable.expectation_value(state)

# def cost_function(args):
#     GROUPS = [list(range(num_qubits))]  # grupos de qubits
#     n_shots = 1024
#     parameters, qpu = args
#     circuit = ansatz.assign_parameters(parameters)

#     # send to QPU and call result
#     counts = qpu.run(circuit, transpile=True, shots=n_shots).result().get_counts()
    
#     pdf = pd.DataFrame.from_dict(counts, orient="index").reset_index()
#     pdf.rename(columns={"index": "state", 0: "counts"}, inplace=True)
#     pdf["probability"] = pdf["counts"] / n_shots 
#     pdf["energy"] = pdf.apply(lambda x: opa(x, GROUPS), axis=1)
#     return sum(pdf["energy"]).real

init_qjobs = []
init_params = np.ones(num_parameters)*np.pi
for q in qpus:
    init_qjobs.append(q.run(ansatz.assign_parameters(init_params), transpile=False, shots=1024))


def cost_function(args):
    GROUPS = [list(range(num_qubits))]  # grupos de qubits
    n_shots = 1024
    parameters, qjob = args
    parameters = parameters.tolist()

    # send to QPU and call result
    counts = qjob.upgrade_parameters(parameters).result().get_counts()
    
    pdf = pd.DataFrame.from_dict(counts, orient="index").reset_index()
    pdf.rename(columns={"index": "state", 0: "counts"}, inplace=True)
    pdf["probability"] = pdf["counts"] / n_shots 
    pdf["energy"] = pdf.apply(lambda x: opa(x, GROUPS), axis=1)
    return sum(pdf["energy"]).real

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

result = differential_evolution(cost_function, bounds, maxiter=1, disp=True, workers=mapper, strategy='best1bin', init=pop)

print(result)


