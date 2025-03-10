#!/bin/bash

if [ ! -z "$1" ]; then
    cd ../
    if [ $LMOD_SYSTEM_NAME == "QMIO" ]; then
        # Execution for QMIO
        ml load qmio/hpc gcc/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0 qiskit/1.2.4-python-3.9.9
        conda deactivate
        
        export INSTALL_PATH=$1
        export PATH=$PATH:$INSTALL_PATH/bin

        cmake -G Ninja -B build/
        ninja -C build -j $(nproc)
        cmake --install build/
    else 
        # Execution for FT3 
        ml load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 flexiblas/3.3.0 boost python/3.10.8 pybind11/2.12.0 cmake/3.27.6

        conda deactivate
        
        export INSTALL_PATH=$1
        export PATH=$PATH:$INSTALL_PATH/bin

        cmake -G Ninja -B build/ cmake -B build/ -DPYBIND_PATH=/opt/cesga/2022/software/Compiler/gcccore/system/pybind11/2.12.0/lib64/python3.9/site-packages/pybind11
        ninja -C build -j $(nproc)
        cmake --install build/
    fi
else
    echo "No install path provided. Please provide an argument." >&2
fi