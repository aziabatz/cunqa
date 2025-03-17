#!/bin/bash
 
#ml load qmio/hpc gcc/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0 qiskit/1.2.4-python-3.9.9

ml load cesga/2022 gcc/system flexiblas/3.3.0 openmpi/5.0.5 boost pybind11 cmake qiskit/1.2.4
export INSTALL_PATH=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/installation
 
export PATH=$PATH:$INSTALL_PATH/bin
 
export INFO_PATH=$STORE/.cunqa/qpus.json
 
IP=$(ip route get 1.2.3.4 | head -1 | awk '{print $7}')
nohup jupyter-notebook --ip $IP &> jup.out &