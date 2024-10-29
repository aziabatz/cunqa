#!/bin/bash
#SBATCH -J myjob 		 # jobname
#SBATCH -o myjob_%j.out 	 # output file
#SBATCH -e myjob_%j.err 	 # error file
#SBATCH -n 1
#SBATCH -c 4 				 # number of cores
#SBATCH -t 02:00:00 		 # time for the job
#SBATCH --mem-per-cpu=2G 	 # memory per core


conda activate qiskit_dask


module load qmio/hpc gcc/12.3.0 impi/2021.13.0