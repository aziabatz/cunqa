from utils.utils import Ising_Hamiltonian, hardware_efficient_ansatz, estimate_observable
from multiprocessing.pool import Pool
from qiskit import transpile
from qiskit_aer import AerSimulator
from qmiotools.integrations.qiskitqmio import FakeQmio
from scipy.optimize import differential_evolution
import numpy as np
import time
import json

# choosing backend
backend = FakeQmio("/opt/cesga/qmio/hpc/calibrations/2025_04_02__12_00_02.json", gate_error=True, thermal_relaxation=True, readout_error = True)

# describin the VQE problem with a Hardware Efficient Ansatz

Hamiltonian = Ising_Hamiltonian(n = 4, J = 1)

parametric_ansatz = hardware_efficient_ansatz(num_qubits=4, num_layers=2)

transpiled_parametric_ansatz = transpile(parametric_ansatz, backend, optimization_level = 3, initial_layout = [15,16,25,26], seed_transpiler = 34)

def cost_function(params):

    assembled_circuit = transpiled_parametric_ansatz.assign_parameters(params)

    job = backend.run(assembled_circuit, shots = 1000)

    result = job.result()

    counts = result.get_counts()

    return estimate_observable(Hamiltonian, counts)


bounds=[]
for i in range(transpiled_parametric_ansatz.num_parameters):
    bounds.append((-np.pi,np.pi))


opt_path = []; params_path = []

def cb(xk, convergence = 1e-50):
    print(f"({time.strftime('%H:%M:%S')})")
    cost = cost_function(xk)
    params_path.append(list(xk))
    opt_path.append(cost)


mapper = Pool(transpiled_parametric_ansatz.num_parameters).map

tick = time.time()

result = differential_evolution(func = cost_function,
                                            popsize=1,
                                            strategy='best1bin',
                                            bounds=bounds,
                                            maxiter=10,
                                            disp=True,
                                            init="halton", # halton, x0
                                            polish=False,
                                            tol=1e-50,
                                            seed=34,
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

with open("results/vqe-pool.json", "w") as f:
    json.dump(result, f, indent=2)