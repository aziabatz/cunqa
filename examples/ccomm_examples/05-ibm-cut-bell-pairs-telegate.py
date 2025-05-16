import os, sys
import numpy as np
# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

from cunqa.qutils import getQPUs, qraise, qdrop
from cunqa.circuit import CunqaCircuit
from cunqa.mappers import run_distributed
from cunqa.qjob import gather

def how_big_a_combination(n):
    print(f"For generating a {n}-Cut Bell Pair Factory one needs to specify {4**(n)-2**(n)+2**(2**n)-1} parameter sets.")
    return 4**(n)-2**(n) + 2**(2**n)-1


# Raise QPUs (allocates classical resources for the simulation job) and retrieve them using getQPUs #
family = qraise(2,"00:10:00", simulator="Cunqa", classical_comm=True, cloud = True)
qpus_QPE  = getQPUs(family)

# Params for the gates in the Cut Bell Pair Factory #
with open("/CUNQA/examples/ccomm_examples/two_qpd_bell_pairs_param_values.txt") as fin:
    params2 = [[float(val) for val in line.replace("\n","").split(" ")] for line in fin.readlines()]
z = params2[0]

############ CIRCUIT 1 #########################
Alice = CunqaCircuit(3,3, id="Alice")
# Cut Bell Pair Factory 1 #
Alice.rz(z[0] + np.pi/2 ,1)   
Alice.rz(z[1] + np.pi/2 ,2)   
#Alice.sx(1)                        SX NOT YET IMPLEMENTED
#Alice.sx(2)
Alice.rz(z[4],1)
Alice.rz(z[5],2)
#Alice.sx(1)
#Alice.sx(2)
Alice.rz(5*np.pi/2 + z[8],1)   
Alice.rz(5*np.pi/2 + z[9],2)   
Alice.cx(1,2)
Alice.rz(z[12],2)
Alice.cx(1,2)
Alice.rz(z[14],1)


# First telegate #
Alice.cx(0,1)
Alice.remote_c_if("z", target_qubits = 0, param=None, control_circuit = "Bob")
Alice.measure_and_send(control_qubit = 1, target_circuit = "Bob")
# Second telegate #
Alice.cx(0,2)
Alice.remote_c_if("z", target_qubits = 0, param=None, control_circuit = "Bob")
Alice.measure_and_send(control_qubit = 2, target_circuit = "Bob")

Alice.measure(0,0)
Alice.measure(1,1)
Alice.measure(2,2)

############# CIRCUIT 2 ######################

Bob = CunqaCircuit(3,3, id="Bob")
# Cut Bell Pair Factory 2 #
Bob.rz(z[2]+ np.pi/2 ,0)   
Bob.rz(z[3]+ np.pi/2 ,1)   
#Bob.sx(0)
#Bob.sx(1)
Bob.rz(z[6],0)
Bob.rz(z[7],1)
#Bob.sx(0)
#Bob.sx(1)
Bob.rz(5*np.pi/2 + z[10],0)   
Bob.rz(5*np.pi/2 + z[11],1)   
Bob.cx(1,0)
Bob.rz(z[13],0)
Bob.cx(1,0)
Bob.rz(z[15],1)

# First telegate #
Bob.cx(1,2)
Bob.h(1)
Bob.measure_and_send(control_qubit = 1, target_circuit = "Alice")
Bob.remote_c_if("x", target_qubits = 2, param=None, control_circuit = "Alice")
# Second telegate #
Bob.cx(0,2)
Bob.h(0)
Bob.measure_and_send(control_qubit = 0, target_circuit = "Alice")
Bob.remote_c_if("x", target_qubits = 2, param=None, control_circuit = "Alice")

Bob.measure(0,0)
Bob.measure(1,1)
Bob.measure(2,2)



circs_QPE = [Alice, Bob]

########## Distributed run + print results ##########
distr_jobs = run_distributed(circs_QPE, qpus_QPE, shots=1024) 
result_list = gather(distr_jobs)

for res in result_list:
    print(res)

# Drop the deployed QPUs #
qdrop(family)