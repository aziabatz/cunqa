#!/bin/bash


#SBATCH -J qraise_from_slurm            # Job name
#SBATCH -o qraise_from_slurm_%j.o       # Name of stdout output file(%j expands to jobId)
#SBATCH -e qraise_from_slurm_%j.e       # Name of stderr output file(%j expands to jobId)
#SBATCH -n 4                 # Number of processes
#SBATCH -N 2
#SBATCH -c 2                # Cores per task requested
#SBATCH --time 00:30:00         # Run time (hh:mm:ss) 
#SBATCH --mem-per-cpu=2G    # Memory per core demandes 

export INSTALL_PATH=/mnt/netapp1/Store_CESGA/home/cesga/acarballido/repos/api-simulator/installation
export PATH=$PATH:$INSTALL_PATH/bin
export INFO_PATH=$STORE/.api_simulator/qpus.json
#export INFO_PATH_2=$STORE/.api_simulator/qpus_2.json

ml load qmio/hpc gcc/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0 qiskit/1.2.4-python-3.9.9

n_qpus_0=1
n_qpus_1=2
time_qpus="00:10:00"
cores_per_qpu=2
mem_per_qpu=20
number_of_nodes=1
simulator="Aer"
# path_to_backend = 


total_tasks=$((n_qpus + 1)) 
mem_per_core=$((mem_per_qpu/cores_per_qpu))

srun -n ${n_qpus_0} --cpus-per-task=${cores_per_qpu} --mem=${mem_per_qpu} --task-epilog=$INSTALL_PATH/bin/epilog.sh setup_qpus ${INFO_PATH} ${simulator} &
srun -n ${n_qpus_0} --cpus-per-task=${cores_per_qpu} --mem=${mem_per_qpu} --task-epilog=$INSTALL_PATH/bin/epilog.sh setup_qpus ${INFO_PATH} ${simulator} &


#python /mnt/netapp1/Store_CESGA/home/cesga/acarballido/repos/api-simulator/examples/example_aer.py

wait


            