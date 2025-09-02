
from qiskit.quantum_info import SparsePauliOp
import os
import sys
installation_path = os.getenv("HOME")
sys.path.append(installation_path)

import numpy as np, warnings

from cunqa import gather

import pandas as pd

np.seterr(divide='raise', invalid='raise')



def Ising_Hamiltonian(n, J, g):
    """
    Ising Hamiltonian of the form: H= J \sum Z_iZ_i+1 +g \sum X_i, where n is the number of qubits.
    """
    Id = Id = "I"*n
    Hx = 0; Hzz = 0
    for i in range(n):
        if i != n-1:
            Hzz += J*SparsePauliOp(Id[:i]+"ZZ"+Id[i+1:-1])
        else: # adding closed cc
            Hzz += J*SparsePauliOp("Z"+Id[:(n-2)]+"Z")
        Hx += g*SparsePauliOp([Id[:i]+"X"+Id[i:-1]])
    return Hzz+Hx

from qiskit import QuantumCircuit
from qiskit.circuit.library import EvolvedOperatorAnsatz


def hva(hamiltonian, layers):
    """
    Creates a Hamiltonian variational ansatz of the number of layers provided, given the Hamiltonian.
    Parametrized "blocks" will be separated by barriers.
    """
    n = hamiltonian.num_qubits
    circuit = QuantumCircuit(n)
    i = 0
    for l in range(layers):
        for H in hamiltonian.group_commuting():
            sample = EvolvedOperatorAnsatz(H,parameter_prefix='theta_['+str(i)+']' )
            circuit.compose(sample, inplace=True)
            i+=1
            # each block is corresponded to one paramter, it is important that in each layer everything commutes
            circuit.barrier()
    return circuit

def layered_circs(circuit):
    """
    Separates the circuit in n circuits that contain 1,2,... n layers of the circuit.
    """
    circuits = []
    qc = QuantumCircuit(circuit.num_qubits)
    for data in circuit.data:
        if data.operation.name != "barrier":
            # print(f"{data.operation.name} is not a barrier...")
            qc.append(data)
        else:
            # print("We found a barrer! Let's append the circuit: ")
            # display(qc.draw('mpl'))
            circuits.append(qc.copy())

    return circuits

import numpy as np
from qiskit.quantum_info import Statevector



def g_layer_statevector(K,circuit):
    state = Statevector.from_instruction(circuit)

    g = state.expectation_value((K @ K)).real - state.expectation_value(K)*state.expectation_value(K).real
            # print(f"For element {(i,j)} we get: <{(observables[i] @ observables[j]).paulis[0].__str__()}> - <{observables[i].paulis[0].__str__()}><{observables[j].paulis[0].__str__()}> = {result}")
    
    return g


def g_layer_shots(K, circuit, shots=3000):
    """
    Calculates g tensor with the expected values of the state preparated by the circuit.

    For n observables, g will be nxn, each element calculated as:
        g_{i,j} = <state|O_i @ O_j|state> - <state|O_i|state><state|O_j|state>

    Note that for i=j we have the variance of the observable respect to the state.
    """
    n_qubits = K.num_qubits

    observables = [K,K@K] # aqui tendria por ejemplo: [PauliOP(IZZ,ZZI),PauliOP(III, ZIZ)]
    # tengo que sacar el valor esperado de esos para construir g
    pauli_strings = 0
    for o in observables:
        for p in o: 
            pauli_strings += p
    
    # junté todas las paulis
    measurement_ops = []
    for group in pauli_strings.paulis.group_qubit_wise_commuting():# hacemos los grupos para sacar el operador de medida de cada uno de ellos
        op='I'*n_qubits
        for element in group.to_labels():
            for n,pauli in enumerate(element):
                if pauli !='I':
                        op=op[:n]+str(pauli)+op[int(n+1):]
        measurement_ops.append(op)
    
    qwc_circuits = []
    for measurement_op in measurement_ops:# para cada operador de medida (qwc group) genero un circuito con las correspondientes rotaciones para medir en las bases
        qp=QuantumCircuit(n_qubits)
        index=1
        for j in measurement_op:
            if j=='Y':
                qp.sdg(n_qubits-index)
                qp.h(n_qubits-index)
            if j=='X':
                qp.h(n_qubits-index)
            index+=1
        qwc_circuits.append(qp)

    for i in qwc_circuits:
        i.compose(circuit,inplace=True, front=True)
        i.measure_all()
    
    # ya tendría mis circuitos! que para el caso de nuestro HVA es solo uno...

    from qiskit_aer import AerSimulator
    from qiskit import transpile
    import pandas as pd

    backend = AerSimulator()

    qwc_circuits_transpiled = transpile(qwc_circuits, backend,seed_transpiler=1, optimization_level=3)

    ev = 0
    counts = []
    for q in qwc_circuits_transpiled:
        job = backend.run(q, shots = shots)
        ev+=1
        result = job.result()
        counts.append(result.counts)

    expected_values = []
    for obs in observables:
        # print(" For observable ", obs)
        for n,paulis in enumerate(measurement_ops):
            # comprobamos que la medida de ese operador se corresponde con el observable
            if len((obs + SparsePauliOp(paulis)).paulis.group_qubit_wise_commuting()) == 1:
                op = n

        # print("measurement operator ", measurement_ops[op])
        
        # selecciono los counts de mi operador
        pdf = pd.DataFrame.from_dict(counts[op], orient="index").reset_index()
        pdf.rename(columns={"index":"state", 0: "counts"}, inplace=True)
        pdf["probability"] = pdf["counts"] / shots 
        pdf["enery"] = pdf.apply(lambda x: opa(x, obs), axis=1)

        expected_value = sum(pdf["enery"]).real

        # print("Expected value ", expected_value)
        # print()

        expected_values.append(expected_value)

    # print("computing g...")
    #print(f"g = {expected_values[1]}-{expected_values[0]*expected_values[0]}")    

    g = expected_values[1] - expected_values[0]*expected_values[0]

    # print("g for layer ", g)

    return g, ev


def g_layer_cunqa(K, circuit, shots, qpus):
    """
    Calculates g tensor with the expected values of the state preparated by the circuit.

    For n observables, g will be nxn, each element calculated as:
        g_{i,j} = <state|O_i @ O_j|state> - <state|O_i|state><state|O_j|state>

    Note that for i=j we have the variance of the observable respect to the state.
    """

    ev = 0
    qjobs = []
    counts = []
    for j, qc in enumerate(qwc_circuits):
        if len(qpus) == 1:
            qpu = qpus[0]
        else:
            qpu = qpus[j]
        qjob = qpu.run(qc, transpile=True,optimization_level=2, shots = shots)
        ev+=1
        qjobs.append(qjob)

    results = gather(qjobs)

    counts = [r.counts for r in results]

    expected_values = []
    for obs in observables:
        # print(" For observable ", obs)
        for n,paulis in enumerate(measurement_ops):
            # comprobamos que la medida de ese operador se corresponde con el observable
            if len((obs + SparsePauliOp(paulis)).paulis.group_qubit_wise_commuting()) == 1:
                op = n

        # print("measurement operator ", measurement_ops[op])
        
        # selecciono los counts de mi operador
        pdf = pd.DataFrame.from_dict(counts[op], orient="index").reset_index()
        pdf.rename(columns={"index":"state", 0: "counts"}, inplace=True)
        pdf["probability"] = pdf["counts"] / shots 
        pdf["enery"] = pdf.apply(lambda x: opa(x, obs), axis=1)

        expected_value = sum(pdf["enery"]).real

        # print("Expected value ", expected_value)
        # print()

        expected_values.append(expected_value)

    # print("computing g...")
    #print(f"g = {expected_values[1]}-{expected_values[0]*expected_values[0]}")    

    g = expected_values[1] - expected_values[0]*expected_values[0]

    # print("g for layer ", g)

    return g, ev


import re
def opa(row, obs):
    coeffs = obs.coeffs
    group = obs.paulis
    suma = 0.000000000
    for string,c in zip(group, coeffs):
        st= re.sub(r"[X,Y,Z]", "1", string.to_label())
        st =  re.sub(r"I", "0", st)
        step = (-1)**sum([int(a) * int(b) for a,b in zip(st, row["state"])]) * row["probability"] * c
        #print(string, st, pdf["state"].iloc[0], step, c)
        suma += step
    return suma.real


from scipy.linalg import block_diag, pinv, inv

def g_tensor(hamil, qc, layers, shots, backup, method = "shots", cunqa = False, qpus=None):
    """
    Takes the hamiltonian and the circuit and calculates g for each layer and then the G tensor of the circuit.
    """
    circuits = layered_circs(qc)

    # print("number of layered circuits: ", len(circuits))
    Ks_one_layer = [sum(h) for h in hamil.group_commuting()]# observables for one layer of HVA, which is not a layer of the QNG ansatz
    Ks = []
    for _ in range(layers):
        for o in Ks_one_layer:
            Ks.append(o)
    
    k=1
    tensors = []
    # print("Number of layered circuits: ", len(circuits[:-1]))
    # print("Number of observables: ", len(observables))
    evals = 0
    for circ, K in zip(circuits[:-1], Ks):
        # print(f"for layer {k}:"); k+=1
        # print("circuit: ")
        # print(circ.decompose())
        # print(obs)
        # print("Layer ", k); k+=1
        # for _,o in enumerate(obs):
        #     print(f"K_{_}={[o.paulis[i].__str__() for i in range(len(o))]}")
        if cunqa:

            g, ev = g_layer_cunqa(K, circ.decompose().decompose().decompose(), shots=shots, qpus=qpus)

        else:

            if method == "shots":
                g,ev = g_layer_shots(K, circ, shots=shots)
                evals+=ev

            elif method == "statevector":
                g = g_layer_statevector(K,circ)
        
        tensors.append(g)
    
    # print(tensors)
    backup_count = 0
    G = block_diag(tuple(tensors))
    G = block_diag(*tensors)
    Gd = np.zeros((len(G),len(G)))
    for i in range(len(G)):
        if G[i][i] == 0:
            print("\tWARNING: zero division detected")
            backup_count += 1
            if backup == "variance":
                Gd[i][i] = 1/np.sqrt(shots)
            elif backup == "freeze":
                Gd[i][i] = 0
            elif backup == "gradient":
                Gd[i][i] = 1
        else:
            Gd[i][i] = 1/G[i][i]

        
    return Gd, backup_count, evals


import numpy as np
def numerical_gradient(executor, cost_fn, params, *args, epsilon=0.05):# aqui ejecutamos el circuito 2*num_params
    """
    Estimates the gradient of the function with respect to the parameters up to the first order approximation.
    """
    grads = np.zeros_like(params)
    for i in range(len(params)):
        delta = np.zeros(len(params))
        delta[i] = epsilon

        qjobs_loss_pluss = executor(params + delta)
        
        loss_plus = cost_fn(params + delta, *args)
        loss_minus = cost_fn(params - delta, *args)
        
        grads[i] = (loss_plus - loss_minus) / (2 * epsilon)

    return grads


def flatten(list_of_lists):
    """
    Flattens a list of lists into a single list.
    """
    return [item for sublist in list_of_lists for item in sublist]
