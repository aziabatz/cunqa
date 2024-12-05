# api-simulator
API definition for communicating the user with the simulator.

# Clone repository
In order to get all the submodules correctly loaded do:

```console
git clone --recursive git@github.com:CESGA-Quantum-Spain/api-simulator.git
cd api-simulator/scripts/
bash setup_submodules.sh
```

# C++ Dependecies
TO BE EXPLICITED

# Installation 
First of all, some modules have to be loaded.

- In the FT3, load the modules:
```console
ml load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 scalapack/2.2.0 openblas/system boost
```
- In the QMIO, **TO BE DETERMINED**.

Now, using the environment path `INSTALL_PATH`, define the directory that you want to install to. **Use absolute paths for a consistent usage**. 

```console
export INSTALL_PATH=<your/installation/path>
```

Once your `INSTALL_PATH` variable has been set, export the next two variables. They will be employed in the execution process of the commands created in the build process (standard practice in C++ installation process).

```console
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_PATH/lib
export PATH=$PATH:$INSTALL_PATH/bin
```

Now everything is set for the installation. 

```console
cmake -B build/
cmake --build build/
cmake --install build/
```




