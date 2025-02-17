# CUNQA
Infraestructure for executing distributed quantum computations employing CESGA resources as emulated QPUs.

## Clone repository
In order to get all the submodules correctly loaded, please remember the `--recursive` option when cloning.

```console
git clone --recursive git@github.com:CESGA-Quantum-Spain/CUNQA.git
```
To ensure all submodules are correctly installed, we encourage to run the *setup_submodules.sh* file:

```console
cd CUNQA/scripts/
bash setup_submodules.sh
```

## Installation 
#### QMIO
###### Automatic configuration
The `scripts/configure.sh` file is prepared to bring an authomatic installation of **CUNQA**. The user only has to execute this file followed by the path to the desire installation folder: 

```console
source configure.sh <your/installation/path>
``` 

If the authomatic installation fails, try to the manual installation.

###### Manual configuration
1. First of all, deactivate miniconda to not interfere with the rest of the installation:

```console
conda deactivate
```

2. Then, load the following modules:

```console
ml load qmio/hpc gcc/system gcccore/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0
```

3. Define the enviroment variable `INSTALL_PATH` as the **ABSOLUTE PATH** to the directory where **CUNQA** wants to be installed. 

```console
export INSTALL_PATH=<your/installation/path>
```

4. Afterwards, add the bin folder to `PATH` in order to correctly execute all the commands to use the platform.

```console
export PATH=$PATH:$INSTALL_PATH/bin
```

5. Once the previous steps are done, everything is set for the build/installation. There are two options: 


**Standard way (slower)**
```console
cmake -B build/ 
cmake --build build/
cmake --install build/
```

**Using ninja (faster)**
```console
cmake -G Ninja -B build/
ninja -C build -j $(nproc)
cmake --install build/
```


#### FINISTERRAE III (FT3)

In the FT3, the installation is almost the same as in QMIO but with few exceptions. 

For the **authomatic configuration**, the process is exactly the same as presented in the corresponding section for QMIO.

In the case of a **manual configuration**, the 1-4 steps are equal as in QMIO but loading the following modules in step 2:

```console
ml load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 flexiblas/3.3.0 boost python/3.10.8 pybind11/2.12.0 cmake/3.27.6
```

For step 5, instead of a simple `cmake -B build/`, the user has to add the `-DPYBIND_DIR` option with the path to the pybind11 cmake modules:

```console
cmake -B build/ -DPYBIND_PATH=/opt/cesga/2022/software/Compiler/gcccore/system/pybind11/2.12.0/lib/python3.9/site-packages/pybind11/share/cmake/pybind11
```

And that's it! Everything is set—either on QMIO or in the FT3—to perform an execution. 

## RUN YOUR FIRST DISTRIBUTED PROGRAM

Once **CUNQA** is installed, the basic workflow to use it is:
1. Raise the desired number of QPUs with the command `qraise`.
2. Run circuits on the raised QPUs:
2.1. Connect to the QPUs through the python script.
2.2. Define the circuits to execute.
2.3. Execute the circuits on the QPUs.
3. Drop the raised QPUs with the command `qdrop`.

Please, note that steps 1-4 of the [Installation section](#installation) have to be donde every time **CUNQA** wants to be used.

### 1. `qraise`command
This command allows to raise as many QPUs as desired. Each QPU can be configured by the user to have a personalized backend. There is a help FLAG with a quick guide of how this command works:
```console
qraise --help
```
1. The only two mandatory FLAGS of `qraise` are the **Number of QPUs**, set up with `-n` or `--num_qpus` and the **Maximum time** the QPUs will be raised, set up with `-t` or `--time`. 
So, for instance, the command 
```console 
qraise -n 4 -t 01:20:30
``` 
will raise four QPUs during at most 1 hour, 20 minutes and 30 seconds.  
> **Note:** By default, all the QPUs will be raised with [AerSimulator](https://github.com/Qiskit/qiskit-aer) as the background simulator and IdealAer as the background backend. **(To Improve)**
2. The simulator and the backend configuration can be configured by the user through `qraise` FLAGs:
>**Set simulator:** 
>```console
>qraise -n 4 -t 01:20:30 --sim=Munich
>```
> The command above changes the default simulator by the [mqt-ddsim simulator](https://github.com/cda-tum/mqt-ddsim). Currently, **CUNQA** only allows two simulators: ``--sim=Aer`` and ``--sim=Munich``.

>**Set FakeQmio:**
>```console
>qraise -n 4 -t 01:20:30 --fakeqmio=<path/to/calibrations/file>
>```
>The `--fakeqmio` FLAG raises QPUs as simulated [QMIO](https://www.cesga.es/infraestructuras/cuantica/)s. If no `<path/to/calibrations/file>` is provided, last calibrations of de QMIO are used.

>**Set personalized backend:**
>```console
>qraise -n 4 -t 01:20:30 --backend=<path/to/backend/json>
>```
>The personalized backend has to be a *json* file with the following structure:
>```json
>{"backend":{"name": "BackendExample", "version": "0.0","simulator": "AerSimulator", "n_qubits": 32,"url": "", "is_simulator": true, "conditional": true, "memory": true, "max_shots": 1000000, "description": "", "basis_gates": [], "custom_instructions": "", "gates": [], "coupling_map": []}, "noise": {}
>```
>**Note:** The "noise" key must be covered with a json with noise supported by the chosen simulator.


### 2. Python Program Example
Once the QPUs are raised, they are ready to execute any quantum circuit. The following script is a basic workflow to do that.

>**Warning:** To execute the following python example it is needed  to load the [Qiskit](https://github.com/Qiskit/qiskit) module:
In QMIO:
>```console 
>module load qmio/hpc gcc/12.3.0 qiskit/1.2.4-python-3.9.9
>```
>In FT3:
>```console 
>module load cesga/2022 gcc/system qiskit/1.2.4
>```


```python 
# Python Script Example

import os
import sys

# Adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)

# Let's get the raised QPUs
from cunqa.qpu import getQPUs

qpus  = getQPUs() # List of raised QPUs
for q in qpus:
    print(f"QPU {q.id}, name: {q.backend.name}, backend: {q.backend.simulator}, version: {q.backend.version}.")

# Let's create a circuit to run in our QPUs!
from qiskit import QuantumCircuit

N_QUBITS = 2 # Number of qubits
qc = QuantumCircuit(N_QUBITS)
qc.h(0)
qc.cx(0,1)
qc.measure_all()

# Time to run
qpu0 = qpu[0] # Select one of the raise QPUs

job = qpu0.run(qc, transpile = True, shots = 1000)

result = job.result() # Get the result of the execution

counts = result.get_counts() 
```

**Note:** It is not mandatory to run a QuantumCircuit from Qiskit. You can also send the circuit in *OpenQASM 2.0* format or in *json* format. The last with the following structure: 
```json
{"instructions": [{"name": "h", "qubits": [0], "params": []},{"name": "cx", "qubits": [0, 1], "params": []}, {"name": "rx", "qubits": [0], "params": [0.39528385768119634]}, {"name": "measure", "qubits": [0], "memory": [0]}], "num_qubits": 2, "num_clbits": 4, "quantum_registers": {"q": [0, 1]}, "classical_registers": {"c": [0, 1], "other_measure_name": [2], "meas": [3]}}

```
### 3. `qdrop` command
Once the work is finished, the raised QPUs should be dropped in order to not monopolize computational resources. 
The `qdrop` command can be used to drop a set of QPUs raised with a single `qraise` simply by passing its `SLURM_JOB_ID`:
```console 
qdrop SLURM_JOB_ID
```
Note that the ```SLURM_JOB_ID``` can be obtained, for instance, executing the `squeue` command.

If the user wants to drop all the raised QPUs, just make:
```console 
qdrop --all
```



