from utils.utils import Ising_Hamiltonian, hardware_efficient_ansatz, estimate_observable
from multiprocessing.pool import Pool
from cunqa import transpiler
from scipy.optimize import differential_evolution
import numpy as np
import time
import json
from cunqa import get_QPUs
from cunqa.mappers import QPUCircuitMapper
from qmiotools.integrations.qiskitqmio import FakeQmio
from qiskit import transpile
backend = FakeQmio("/opt/cesga/qmio/hpc/calibrations/2025_04_02__12_00_02.json", gate_error=True, thermal_relaxation=True, readout_error = True)


# finding the virtual QPUs
qpus = get_QPUs(local=False)

# describin the VQE problem with a Hardware Efficient Ansatz

Hamiltonian = Ising_Hamiltonian(n = 10, J = 1)

parametric_ansatz = hardware_efficient_ansatz(num_qubits=10, num_layers=2)

transpiled_parametric_ansatz = transpile(parametric_ansatz, backend, optimization_level = 3, initial_layout = [19,20,23,30,29,28,31,22,21,13], seed_transpiler = 34)

print(transpiled_parametric_ansatz.num_parameters)
# now the cost function works with Result objects since the mapper will do the sending and gathering of the QJobs

def cost_function(result):

    counts = result.counts

    tick = time.time()
    obs = estimate_observable(Hamiltonian, counts)
    tack = time.time()

    print("Estimation time: ", tack-tick)

    return obs


bounds=[]
for i in range(transpiled_parametric_ansatz.num_parameters):
    bounds.append((-np.pi,np.pi))


opt_path = []; params_path = []

def cb(xk, convergence = 1e-50):
    print(f"({time.strftime('%H:%M:%S.%f')})")
    cost = mapper(cost_function, [xk])[0]
    params_path.append(list(xk))
    opt_path.append(cost)

mapper = QPUCircuitMapper(qpus, transpiled_parametric_ansatz, shots = 1e4, seed = 34)

tick = time.time()

result = differential_evolution(func = cost_function,
                                            popsize=1,
                                            strategy='best1bin',
                                            bounds=bounds,
                                            maxiter=1000,
                                            disp=True,
                                            init="halton", # halton, x0
                                            polish=False,
                                            tol=1e-50,
                                            seed=33,
                                            callback=cb,
                                            updating='deferred',
                                            workers = mapper)


tack = time.time()

if i <= 300:
    best_cost = result.fun
else:
    best_cost = np.mean(opt_path[-100:])

best_params = list(result.x)
time_taken = tack-tick

result = {
    "best_cost":best_cost,
    "best_params":best_params,
    "opt_path":opt_path,
    "params_path":params_path,
    "time_taken":time_taken,
    "n_steps":len(opt_path)
}

with open("results/vqe-cunqa.json", "w") as f:
    json.dump(result, f, indent=2)