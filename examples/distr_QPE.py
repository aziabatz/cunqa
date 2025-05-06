import os, sys
import numpy as np
# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

from cunqa.qutils import getQPUs, qraise, qdrop
from cunqa.circuit import CunqaCircuit
from cunqa.mappers import run_distributed
from cunqa.qjob import gather


# Raise QPUs (allocates classical resources for the simulation job) and retrieve them using getQPUs
family = qraise(2,"00:10:00", simulator="Cunqa", classical_comm=True, cloud = True)
os.system('sleep 15')
qpus_QPE  = getQPUs(family)

def distr_rz2_QPE(n_precision):
    """"
    Function for distributed quantum phase estimation.

    Args
    ---------
    n_precision (str): number of digits of the phase to extract
    """
    circuits = {}
    for i in range(n_precision): 
        circuits[f"cc_{i}"] = CunqaCircuit(3,3, id= f"cc_{i}") #we set the same number of quantum and classical bits because Cunqasimulator requires all qubits to be measured for them to be represented on the counts
        circuits[f"cc_{i}"].h(0)




# Helper function, eigenstates and actual phase for each eigenstate
def binary_to_float(binary_str):
    """Convert a binary string with decimal part to float"""
    int_part, dec_part = binary_str.split('.')
    integer = int(int_part, base=2) if int_part else 0
    decimal = sum(int(bit) * 2**(-i-1) for i, bit in enumerate(dec_part))
    return integer + decimal

# Eigenstate |11>
x1 = np.pi
x2 = np.pi
# Eigenstate |00>
#x1 = 0
#x2 = 0
# Eigenstate |01>
#x1 = 0
#x2 = np.pi

phi = binary_to_float("0.11111000100110101011100100001101111")
theta = 2 * np.pi * phi

if x1 == 0 and x2 == 0:            # State |00>
    phase = -phi
elif x1 == np.pi and x2 == np.pi:  # State |11>
    phase = phi
elif x1 == 0 or x2 == 0:           # State |01> or |10>
    phase = 0

# Define the circuits to be run 
cc_1 =CunqaCircuit(3, 3, id="first")

cc_1.h(0)
cc_1.rx(x1,1)
cc_1.rx(x2,2)
cc_1.crz(theta,0,1)
cc_1.crz(theta,0,2)
cc_1.h(0)
cc_1.send_gate("rz",  np.pi*1/2, control_qubit = 0, target_qubit = 0, target_circuit = "second")
cc_1.measure(0,0)
cc_1.measure(1,1)
cc_1.measure(2,2)


cc_2 =CunqaCircuit(3, 3, id="second")

cc_2.h(0)
cc_2.rx(x1,1)
cc_2.rx(x2,2)
cc_2.crz(theta,0,1)
cc_2.crz(theta,0,2)
cc_2.recv_gate("rz", np.pi*1/2, control_qubit = 0, control_circuit = "first", target_qubit = 0)
cc_2.h(0)
cc_2.measure(0,0)
cc_2.measure(1,1)
cc_2.measure(2,2)


circs_QPE = [cc_1, cc_2]

# Run the QPE, each circuit on a different QPU. Get a list with the counts of each circuit and pick the binary string which appears most often
distr_jobs = run_distributed(circs_QPE, qpus_QPE) 

counts_list = [result.get_counts() for result in gather(distr_jobs)]
print(counts_list)

estimation = ''

for counts in counts_list:
    max_key = max(counts, key=counts.get) #finds the element with the highest count and extracts its binary string

    # TODO: Check if it's necessary to reverse or something

    estimation += max_key[0] #concatenate binary string to get the full estimation. 


# Let the user know the estimated phase
print(f"The estimated phase for the gate is: {estimation}.")

qdrop(family)