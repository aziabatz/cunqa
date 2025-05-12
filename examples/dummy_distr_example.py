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

# Define the circuits to be run 
cc_1 =CunqaCircuit(3, 3, id="first")

cc_1.h(0)
cc_1.send_gate("x", param=None, control_qubit = 0, target_qubit = 0, target_circuit = "second")
cc_1.measure(0,0)
cc_1.measure(1,1)
cc_1.measure(2,2)


cc_2 =CunqaCircuit(3, 3, id="second")

cc_2.recv_gate("x", param=None, control_qubit = 0, control_circuit = "first", target_qubit = 0)
cc_2.measure(0,0)
cc_2.measure(1,1)
cc_2.measure(2,2)


circs_QPE = [cc_1, cc_2]

# Run the QPE, each circuit on a different QPU. Get a list with the counts of each circuit and pick the binary string which appears most often
distr_jobs = run_distributed(circs_QPE, qpus_QPE) 

counts_list = [result.get_counts() for result in gather(distr_jobs)]
print(counts_list)

qdrop(family)