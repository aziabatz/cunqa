# api-simulator
API definition for communicating the user with the simulator.

# Clone repository
In order to get all the submodules correctly loaded do:

```console
git clone --recursive git@github.com:CESGA-Quantum-Spain/api-simulator.git
bash scripts/setup_submodules.sh
```

# C++ Dependecies
The list of modules employed (available in the QMIO partition).

- gcc/12.3.0
- boost/1.85.0
- nlohmann_json/3.11.3
- impi/2021.13.0

# Installation

```console
ml load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 scalapack/2.2.0 openblas/system boost

export INSTALL_PATH=<your/installation/path>
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_PATH/lib
export PATH=$PATH:$INSTALL_PATH/bin

cmake -B build/
cmake --build build/
cmake --install build/
```


