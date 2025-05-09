import os, sys
import numpy as np
import matplotlib.pyplot as plt
# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

from cunqa.qutils import getQPUs, qraise, qdrop
from cunqa.circuit import CunqaCircuit
from cunqa.mappers import run_distributed
from cunqa.qjob import gather

def print_results(counts_list):
    estimation = ""
    for counts in counts_list:
        # Extract the most frequent measurement (the best estimate of theta)
        most_frequent_output = max(counts, key=counts.get)
        total = sum([v for v in counts.values()])
        estimation += most_frequent_output[0]
    # Convert the (reversed) binary string to a fraction
    estimated_theta = int(estimation[::-1], 2) / (2 ** len(estimation))

    print(f"Measured output: {estimation[::-1]}")
    print(f"Estimated theta: {estimated_theta}")
    #print(f"With probability: {counts[most_frequent_output]/total}", )




def distr_rz2_QPE(angle, n_precision):
    """"
    Function for distributed quantum phase estimation.

    Args
    ---------
    n_precision (str): number of digits of the phase to extract. We will create the same number of circuits that run in different QPUs to run the distributed QPE.
    """
    family = qraise(n_precision,"00:10:00", simulator="Cunqa", classical_comm=True, cloud = True)

    os.system('sleep 10')

    qpus_QPE  = getQPUs(family)

    circuits = {}
    for i in range(n_precision): 
        theta = angle*np.pi*2**(n_precision-i)
        print(f"Theta: {theta}")

        circuits[f"cc_{i}"] = CunqaCircuit(3,3, id= f"cc_{i}") #we set the same number of quantum and classical bits because Cunqasimulator requires all qubits to be measured for them to be represented on the counts
        circuits[f"cc_{i}"].h(0)
        circuits[f"cc_{i}"].rx(np.pi,1)
        circuits[f"cc_{i}"].rx(np.pi,2)
        circuits[f"cc_{i}"].crz(theta,0,1)
        circuits[f"cc_{i}"].crz(theta,0,2)
        

        for j in range(i):
            print(f"Recibimos de {j} en {i}.")
            circuits[f"cc_{i}"].recv_gate("rz", -np.pi*2**(i-j-2), control_qubit = 0, control_circuit = f"cc_{j}", target_qubit = 0)

        circuits[f"cc_{i}"].h(0)

        for k in range(n_precision-i-1):
            print(f"Mandamos desde {i} a {i+1+k}.")
            circuits[f"cc_{i}"].send_gate("rz", -np.pi*2**(-i+k-1), control_qubit = 0, target_qubit = 0, target_circuit = f"cc_{i+1+k}") 

        circuits[f"cc_{i}"].measure(0,0)
        circuits[f"cc_{i}"].measure(1,1)
        circuits[f"cc_{i}"].measure(2,2)

    #print({"id":circuits["cc_2"]._id, "instructions":circuits[f"cc_{2}"].instructions, "num_qubits": circuits[f"cc_{2}"].num_qubits,"num_clbits": circuits[f"cc_{2}"].num_clbits,"classical_registers": circuits[f"cc_{2}"].classical_regs,"quantum_registers": circuits[f"cc_{2}"].quantum_regs, "exec_type":"dynamic", "is_distributed":circuits[f"cc_{2}"].is_distributed, "is_parametric":circuits[f"cc_{2}"].is_parametric})


    
    distr_jobs = run_distributed(list(circuits.values()), qpus_QPE, shots=2000)
    
    counts_list = [result.get_counts() for result in gather(distr_jobs)]
    print(counts_list)
    print_results(counts_list)
    

    qdrop(family)
        
        

#distr_rz2_QPE(0.25, 8)

distr_rz2_QPE(0.63, 10)


