# CUNQA
API definition for communicating the user with the simulator.

## Clone repository
In order to get all the submodules correctly loaded do:

```console
git clone --recursive git@github.com:CESGA-Quantum-Spain/CUNQA.git
cd CUNQA/scripts/
bash setup_submodules.sh
```

## Installation 
#### QMIO
###### Automatic configuration
One can use the `configure.sh` script in the `scripts/` folder.

```console
source configure.sh <your/installation/path>
``` 

This automatically creates all the environment variables and installs the API and its functionalities.

###### Manual configuration
First of all, some modules have to be loaded and, also, if miniconda is activate, deactivate it to not interfere.

```console
ml load qmio/hpc gcc/system gcccore/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0 impi/2021.13.0
```

Now, using the environment path `INSTALL_PATH`, define the directory that you want to install to. 

**USE ABSOLUTE PATHS FOR A CONSISTENT USAGE**. 

```console
export INSTALL_PATH=<your/installation/path>
```

Once your `INSTALL_PATH` variable has been set, export the bin folder to `PATH` in order to execute correctly the `qraise` and, inside of it, the `setup-qpus`.

```console
export PATH=$PATH:$INSTALL_PATH/bin
```

Now everything is set for the installation. There are two options: 

```console
# Without ninja
cmake -B build/
cmake --build build/
cmake --install build/

# With ninja
cmake -G Ninja -B build/
ninja -C build -j $(nproc)
cmake --install build/
```

We recommend using the Ninja build since it can reduce the compilation up to half of the time. 

#### FINISTERRAE III (FT3)

In the FT3, the installation is exactly the same, but with few exceptions. If the configuration file `configure.sh` is desired to be used for FT3 configuration, modify the file by uncommenting the lines below `## Execution for FT3` and comment the ones below `## Execution for QMIO`.

In the case of a manual configuration for FT3, first, load the following modules instead of the QMIO ones.

```console
ml load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 flexiblas/3.3.0 boost python/3.10.8 pybind11/2.12.0 cmake/3.27.6
conda deactivate
```

Moreover, instead of a simple `cmake -B build/`, the user has to add the `-DPYBIND_DIR` option with the path to the pybind11 cmake modules.

```console
cmake -B build/ -DPYBIND_PATH=/opt/cesga/2022/software/Compiler/gcccore/system/pybind11/2.12.0/lib/python3.9/site-packages/pybind11/share/cmake/pybind11
```

And that's it! Everything is set—either on QMIO or in the FT3—to perform an execution.

