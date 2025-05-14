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
qpus_QPE  = getQPUs(family)


########## Circuits to run ##########
########## First circuit ############
cc_1 = CunqaCircuit(1, 1, id="first")
cc_1.h(0)
cc_1.measure_and_send(control_qubit = 0, target_circuit = "second")
#cc_1.measure(0,0)

########## Second circuit ###########
cc_2 = CunqaCircuit(1, 1, id="second")
cc_2.remote_c_if("x", target_qubits = 0, param=None, control_circuit = "first")
cc_2.measure(0,0)


########## List of circuits #########
circs_QPE = [cc_1, cc_2]


########## Distributed run ##########
distr_jobs = run_distributed(circs_QPE, qpus_QPE, shots=1024) 

########## Collect the counts #######
counts_list = [[circ_res[0],circ_res[1].get_counts()] for circ_res in gather(distr_jobs, True)]

########## Print the counts #######
print(counts_list)

########## Drop the deployed QPUs #
qdrop(family)
