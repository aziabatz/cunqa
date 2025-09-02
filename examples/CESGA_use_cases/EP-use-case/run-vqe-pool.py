
from utils.utils import Ising_Hamiltonian
from utils.differential_evolution import DifferentialEvolutionHVA
from multiprocessing import Pool
from concurrent.futures import ThreadPoolExecutor
import json

# number of qubits
n = 4

# Transverse field
g = 0.5
J = 1

Hamil = Ising_Hamiltonian(n, J, g)

optimizer = DifferentialEvolutionHVA(Hamiltonian=Hamil,
                                     layers = 2,
                                     shots = 1024,
                                     method = "shots",
                                     backend = "fakeqmio")

mapper = Pool(optimizer.num_parameters)
print("number of threads = ", optimizer.num_parameters)

result = optimizer.optimize(init_params=None,
                            bounds=None,
                            max_iter=100,
                            defered=True,
                            mapper=mapper,
                            pop=1,
                            )

print("Pool executor")
print("best cost ", result["best_cost"])
print("time taken ", result["time_taken"])

output_data = result

filename = f"data/fq_pool_shots_n{n}_l{optimizer.layers}.json"
with open(filename, "w") as f:
    json.dump(output_data, f)

