<h1 style="font-size:8em;" style="text-align: center;"> CUNQA </h1>
<h2> A platform to simulate distributed quantum computing on CESGA HPC enviroment. </h2>


<h2> Authors </h2>
<ul>
  <li> <a href="https://github.com/martalosada">Marta Losada Estévez </a> </li>
  <li> <a href="https://github.com/jorgevazquezperez">Jorge Vázquez Pérez </a></li>
  <li> <a href="https://github.com/alvarocarballido">Álvaro Carballido Costas </a></li>
  <li> <a href="https://github.com/D-Exposito">Daniel Expósito Patiño </a></li>
  <br> 
</ul>

<a href="https://www.cesga.es/"><img src="docs/images/cesga_logo.png" alt="CESGA Logo" width="200" height="60"></a> 
<a href="https://quantumspain-project.es/"><img src="docs/images/quantumspain_logo.png" alt="QuantumSpain Logo" width="220" height="45"></a>
<br> 
<br>

# Table of contents
  - [CLONE REPOSITORY](#clone-repository)
  - [INSTALLATION](#installation)
    - [QMIO](#qmio)
      - [Automatic installation](#automatic-installation)
      - [Manual installation](#manual-installation)
    - [FINISTERRAE III (FT3)](#finisterrae-iii-ft3)
      - [Automatic installation](#automatic-installation-1)
      - [Manual installation](#manual-installation-1)
  - [RUN YOUR FIRST DISTRIBUTED PROGRAM](#run-your-first-distributed-program)
    - [1. `qraise`command](#1-qraisecommand)
    - [2. Python Program Example](#2-python-program-example)
    - [3. `qdrop` command](#3-qdrop-command)
  - [ACKNOWLEDGEMENTS](#acknowledgements)


## Clone repository
It is important to say that, for ensuring a correct cloning of the repository, the SSH is the one preferred. In order to get this to work one has to do:

```console
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/SSH_KEY
```
 
Now, everything is set to get the source code. 
> ⚠️ **Warning:**
> In order to get all the submodules correctly loaded, please remember the `--recursive` option when cloning.

```console
git clone --recursive git@github.com:CESGA-Quantum-Spain/cunqa.git
```

As a additional step, we encourage to run the *setup_submodules.sh* file, which removes some of the files unused in the submodules and makes the repository lighter:

```console
cd scripts
bash setup_submodules.sh
```

## Installation 
### QMIO
#### Automatic installation
The `scripts/configure.sh` file is prepared to bring an automatic installation of the **CUNQA** platform. The user only has to execute this file followed by the path to the desire installation folder: 

```console
source configure.sh
``` 

If the automatic installation fails, try the manual installation.

#### Manual installation

1. First of all, deactivate miniconda to not interfere with the rest of the installation:

```console
conda deactivate
```

2. Then, load the following modules:

```console
ml load qmio/hpc gcc/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0 qiskit/1.2.4-python-3.9.9
```

3. Once the previous steps are done, everything is set for the build/installation. There are two options: 
    
* **Standard way (slower)**
```console
cmake -B build/ 
cmake --build build/
cmake --install build/
```

* **Using [Ninja](https://ninja-build.org/) (faster)**
```console
cmake -G Ninja -B build/
ninja -C build -j $(nproc)
cmake --install build/
```
> ❗ **Important:**
> Is **KEY** that this compilation is done in a compute node with sufficient resources (if not, the compilation process could be killed).

### Finisterrae III (FT3)

In the FT3, the installation is almost the same as in QMIO but with few exceptions. 

#### Automatic installation
For the **automatic installation**, the process is exactly the same as the one presented for QMIO:

```console
source configure.sh
``` 

#### Manual installation
In the case of a **manual installation**, the steps are analogous to the shown above for QMIO:

1. Conda deactivation:

```console
conda deactivate
```

2. Loading modules:

```console
ml load cesga/2022 gcc/system flexiblas/3.3.0 openmpi/5.0.5 boost pybind11 cmake qiskit/1.2.4
```

5. Again: configure, compile and install using CMake:

* **Standard way (slower)**
```console
cmake -B build/ 
cmake --build build/
cmake --install build/
```

* **Using [Ninja](https://ninja-build.org/) (faster)**
```console
cmake -G Ninja -B build/
ninja -C build -j $(nproc)
cmake --install build/
```

> ❗ **Important:**
> Is **KEY** that this compilation is done in a compute node with sufficient resources (if not, the compilation process could be killed).

And that's it! Everything is set—either on QMIO or in the FT3—to perform an execution. 

## Run your first distributed program

Once **CUNQA** is installed, the basic workflow to use it is:
1. Raise the desired QPUs with the command `qraise`.
2. Run circuits on the QPUs:
    - Connect to the QPUs through the python API.
    - Define the circuits to execute.
    - Execute the circuits on the QPUs.
    - Obtain the results.
3. Drop the raised QPUs with the command `qdrop`.
> ❗ **Important:**
> Please, note that steps 1-4 of the [Installation section](#installation) have to be done every time **CUNQA** wants to be used.

### 1. `qraise` command
The `qraise` command raises as many QPUs as desired. Each QPU can be configured by the user to have a personalized backend. There is a help FLAG with a quick guide of how this command works:
```console
qraise --help
```
1. The only two mandatory FLAGS of `qraise` are the **number of QPUs**, set up with `-n` or `--num_qpus` and the **maximum time** the QPUs will be raised, set up with `-t` or `--time`. 
So, for instance, the command 
```console 
qraise -n 4 -t 01:20:30
``` 
will raise four QPUs during at most 1 hour, 20 minutes and 30 seconds. The time format is `hh:mm:ss`.
> 📘 **Note:**
> By default, all the QPUs will be raised with [AerSimulator](https://github.com/Qiskit/qiskit-aer) as the background simulator and IdealAer as the background backend. That is, a backend of 32 qubits, all connected and without noise.
2. The simulator and the backend configuration can be set by the user through `qraise` FLAGs:

**Set simulator:** 
```console
qraise -n 4 -t 01:20:30 --sim=Munich
```
The command above changes the default simulator by the [mqt-ddsim simulator](https://github.com/cda-tum/mqt-ddsim). Currently, **CUNQA** only allows two simulators: ``--sim=Aer`` and ``--sim=Munich``.

**Set FakeQmio:**
```console
qraise -n 4 -t 01:20:30 --fakeqmio=<path/to/calibrations/file>
```
The `--fakeqmio` FLAG raises the QPUs as simulated [QMIO](https://www.cesga.es/infraestructuras/cuantica/)s. If no `<path/to/calibrations/file>` is provided, last calibrations of de QMIO are used. With this FLAG, the background simulator is AerSimulator.

**Set personalized backend:**
```console
qraise -n 4 -t 01:20:30 --backend=<path/to/backend/json>
```
The personalized backend has to be a *json* file with the following structure:
```json
{"backend":{"name": "BackendExample", "version": "0.0", "n_qubits": 32,"url": "", "is_simulator": true, "conditional": true, "memory": true, "max_shots": 1000000, "description": "", "basis_gates": [], "custom_instructions": "", "gates": [], "coupling_map": []}, "noise": {}}
```
> 📘 **Note:**
> The "noise" key must be filled with a json with noise instructions supported by the chosen simulator.

> ❗ **Important:**
> Several `qraise` commands can be executed one after another to raise as many QPUs as desired, each one having its own configuration, independently of the previous ones. The `get_QPUs()` method presented in the section below will collect all the raised QPUs.

### 2. Python Program Example
Once the QPUs are raised, they are ready to execute any quantum circuit. The following script shows a basic workflow.

> ⚠️ **Warning:**
> To execute the following python example it is needed  to load the [Qiskit](https://github.com/Qiskit/qiskit) module:

In QMIO:
```console 
module load qmio/hpc gcc/12.3.0 qiskit/1.2.4-python-3.9.9
```

In FT3:
```console 
module load cesga/2022 gcc/system qiskit/1.2.4
```


```python 
# Python Script Example

import os
import sys

# Adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)

# Let's get the raised QPUs
from cunqa.qutils import get_QPUs

qpus  = get_QPUs() # List of raised QPUs
for q in qpus:
    print(f"QPU {q.id}, name: {q.backend.name}, backend: {q.backend.simulator}, version: {q.backend.version}.")

# Let's create a circuit to run in our QPUs
from qiskit import QuantumCircuit

N_QUBITS = 2 # Number of qubits
qc = QuantumCircuit(N_QUBITS)
qc.h(0)
qc.cx(0,1)
qc.measure_all()

# Time to run
qpu0 = qpus[0] # Select one of the raise QPUs

job = qpu0.run(qc, transpile = True, shots = 1000)

result = job.result() # Get the result of the execution

counts = result.get_counts() 
print(f"Counts: {counts}" )
```

> 📘 **Note:**
> It is not mandatory to run a *QuantumCircuit* from Qiskit. The `.run` method also supports *OpenQASM 2.0* with the following structure:
```json
{"instructions":"OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[2];\ncreg c[2];\nh q[0];\ncx q[0], q[1];\nmeasure q[0] -> c[0];\nmeasure q[1] -> c[1];" , "num_qubits": 2, "num_clbits": 4, "quantum_registers": {"q": [0, 1]}, "classical_registers": {"c": [0, 1], "other_measure_name": [2], "meas": [3]}}

```

and *json* format with the following structure: 
```json
{"instructions": [{"name": "h", "qubits": [0], "params": []},{"name": "cx", "qubits": [0, 1], "params": []}, {"name": "rx", "qubits": [0], "params": [0.39528385768119634]}, {"name": "measure", "qubits": [0], "memory": [0]}], "num_qubits": 2, "num_clbits": 4, "quantum_registers": {"q": [0, 1]}, "classical_registers": {"c": [0, 1], "other_measure_name": [2], "meas": [3]}}

```
### 3. `qdrop` command
Once the work is finished, the raised QPUs should be dropped in order to not monopolize computational resources. 

The `qdrop` command can be used to drop all the QPUs raised with a single `qraise` by passing the corresponding qraise `SLURM_JOB_ID`:
```console 
qdrop SLURM_JOB_ID
```
Note that the ```SLURM_JOB_ID``` can be obtained, for instance, executing the `squeue` command.

To drop all the raised QPUs, just execute:
```console 
qdrop --all
```

## Acknowledgements
