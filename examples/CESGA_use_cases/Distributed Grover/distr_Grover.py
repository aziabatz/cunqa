"""
 Title: Distributed Grover Algorithm.
 Description: implementation of the distributed Gover algorithm from https://arxiv.org/abs/2502.19118 using CUNQA.

Created 14/07/2025
@author: dexposito
"""

import os, sys
from typing import  Union, Any, Optional

# path to access c++ files
sys.path.append(os.getenv("HOME"))

from cunqa.circuit import CunqaCircuit
from cunqa.logger import logger
from cunqa.qpu import QPU
from cunqa.mappers import run_distributed
from cunqa.qutils import qraise, qdrop, get_QPUs
from cunqa.qjob import QJob, gather

def distrGrover(target, n_nodes: int, qubits_per_circuit: list[int], n_layers: int):
    """
    Distributed Grover algorithm presented in https://arxiv.org/abs/2502.19118. 

    Args:
        target (str): bitstring with the state to be targeted, needed to build the oracle
        n_nodes (int): number of nodes through which to distribute the algorithm
        qubits_per_node (list[int]): list specifying how many qubits are on each node
        n_layers (int): specifies how many passes of the two grover blocks (oracle and diffusor) should be applied
    
    Return:

    """
    if not n_nodes == len(qubits_per_circuit):
        logger.error(f"The lenght of the qubits_per_circuit list ({len(qubits_per_circuit)}) does not match the specified number of circuits ({n_nodes}).")
        raise SystemExit

    if not len(target) == sum(qubits_per_circuit):
        logger.error(f"The total number of qubits of the qubits_per_circuit list ({sum(qubits_per_circuit)}) does not match the lenght of the target bitstring ({len(target)}).")
        raise SystemExit

    ###################### CREATE CIRCUITS ######################

    router = CunqaCircuit(n_nodes, id="router")

    circuits = {}
    for i in range(n_nodes):
        circuits[f"circ_{i}"] = CunqaCircuit(qubits_per_circuit[i]+1, qubits_per_circuit[i]+1, id=f"circ_{i}")
        
        for j in range(qubits_per_circuit[i]): 
            circuits[f"circ_{i}"].x(j+1) # Exclude first qubit on each circuit as it is the communication one
            circuits[f"circ_{i}"].h(j+1)

    print(circuits)

    for _ in range(n_layers): # The Oracle and Diffusion blocks are repeated n_layers times

        ###################### ORACLE BLOCK ######################

        for i in range(n_nodes):
            for j in range(qubits_per_circuit[i]):

                if target[sum(qubits_per_circuit[:i]) + j] == '0': # Guille applies the x gate if he finds zeroes, maybe ask him why
                    circuits[f"circ_{i}"].x(j+1)

        distrMCZ(router, circuits)

        for i in range(n_nodes):
            for j in range(qubits_per_circuit[i]):

                if target[sum(qubits_per_circuit[:i]) + j] == '0': 
                    circuits[f"circ_{i}"].x(j+1)

        ###################### DIFFUSION BLOCK ######################

        for i in range(n_nodes):
            for j in range(qubits_per_circuit[i]):
                circuits[f"circ_{i}"].h(j+1)

            distrMCZ(router, circuits)

            for j in range(qubits_per_circuit[i]):
                circuits[f"circ_{i}"].h(j+1)



    for i in range(n_nodes):
        for j in range(qubits_per_circuit[i]):
            circuits[f"circ_{i}"].measure(j,j)





    ###################### EXECUTION PART ######################

    # Raise the required QPUs
    qpus_to_drop = qraise(n_nodes+1, "00:10:00", cloud=True, quantum_comm=True)
    qpus_Grover = get_QPUs(local=False)
    print("before run_distributed")

    # Distributed run
    distr_jobs = run_distributed([router] + list(circuits.values()), qpus_Grover, shots=1000) 
    result_list = gather(distr_jobs)
    print("after run_distributed")
    
    # Print counts
    for result in result_list:
        print(result)

    # drop the deployed QPUs
    qdrop(qpus_to_drop)


def distrMCZ(router, circs):
    for i in range(len(circs)):
        # Here we create entanglement between the first qubit of circuit i and qubit i of the router
        with router.expose(i, circs[f"circ_{i}"]) as rcontrol:
            router.h(i)
            circs[f"circ_{i}"].cx(rcontrol, 0)
        
        router.h(i)
        all_qubits = list(range(circs[f"circ_{i}"].num_qubits))
        circs[f"circ_{i}"].multicontrol(base_gate = "z", num_ctrl_qubits = len(all_qubits)-1, qubits = all_qubits)

        circs[f"circ_{i}"].h(0) # Used to change the measure basis to X
        circs[f"circ_{i}"].measure_and_send(qubit = 0, target_circuit = router) 
        router.remote_c_if("x", qubits = i, control_circuit = f"circ_{i}")

    router.multicontrol(base_gate = "z", num_ctrl_qubits = router.num_qubits-1, qubits = list(range(router.num_qubits)))

    for i in range(len(circs)):
        router.h(i) # Used to change the measure basis to X
        router.measure_and_send(qubit = i, target_circuit = f"circ_{i}")
        all_but_first_qubits =  list(range(circs[f"circ_{i}"].num_qubits))[1:]
        circs[f"circ_{i}"].remote_c_if("mcz", qubits = all_but_first_qubits, control_circuit = router, num_ctrl_qubits = circs[f"circ_{i}"].num_qubits - 2 )

        # Reseting qubits to entangle again
        circs[f"circ_{i}"].reset(0) 
        router.reset(i)





