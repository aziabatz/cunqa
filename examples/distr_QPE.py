import os, sys
# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

from cunqa.qutils import getQPUs, qraise, qdrop
from cunqa.circuit import CunqaCircuit
from cunqa.mappers import run_distributed
from cunqa.qjob import gather


# Raise QPUs (allocates classical resources for the simulation job) and retrieve them using getQPUs
family = qraise(3,"00:10:00", simulator="Cunqa", classical_comm=True, mode="cloud", family_name="for_distr_QPE")
os.system("")
qpus_QPE  = getQPUs(family)

# Define the circuits to be run 
circ_QPE_1 =CunqaCircuit(1)
circ_QPE_2 =CunqaCircuit(1)

circs_QPE = [circ_QPE_1, circ_QPE_2]

# Run the QPE, each circuit on a different QPU. Get a list with the counts of each circuit and pick the binary string which appears most often
distr_jobs = run_distributed(circs_QPE, qpus_QPE)

counts_list = [result.get_counts() for result in gather(distr_jobs)]
estimation = ''

for counts in counts_list:
    bit_string = max(counts, key=lambda x: x[1])[0] #finds the element with the highest count and extracts its binary string
    # TODO: Check if it's necessary to reverse or something
    estimation.append(bit_string) #concatenate binary string to get the full estimation. 


# Let the user know the estimated phase
print(f"The estimated phase for the gate is: {estimation}.")

qdrop(family)