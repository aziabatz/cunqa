import os
import json
import re
import matplotlib.pyplot as plt

results_dir = "results"
modes = [d for d in os.listdir(results_dir) if os.path.isdir(os.path.join(results_dir, d))]

print(modes)

plt.figure(figsize=(8, 6))

for mode in modes:
    mode_path = os.path.join(results_dir, mode)
    qubits = []
    times = []
    cpus = []
    for fname in os.listdir(mode_path):
        if fname.endswith(".json"):
            match = re.match(r"vqe-{}_(\d+)\.json".format(re.escape(mode)), fname)
            if not match:
                continue
            num_qubits = int(match.group(1))
            with open(os.path.join(mode_path, fname), "r") as f:
                data = json.load(f)
            time_taken = data.get("simulation_time")
            if time_taken is not None:
                qubits.append(num_qubits)
                times.append(time_taken)
                cpus.append(4 * num_qubits)
    if qubits and times and cpus:
        qubits, times, cpus = zip(*sorted(zip(qubits, times, cpus)))
        plt.plot(cpus, times, marker='o', label=mode)

# Eje x principal: CPUs
plt.xlabel("Number of qubits")
# Eje x secundario: Qubits
ax = plt.gca()
secax = ax.secondary_xaxis('top', functions=(lambda x: x / 4, lambda x: x * 4))
secax.set_xlabel("Number of CPUs")

plt.yscale("log")
plt.ylabel("Tiempo total (s)")
plt.title("Tiempo vs Número de CPUs por modo de simulación")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
plt.savefig("time_nqubits.png", dpi = 400)