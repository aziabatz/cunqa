#!/bin/bash
# SBATCH -J myjob            # Job name
# SBATCH -o myjob_%j.o       # Name of stdout output file(%j expands to jobId)
# SBATCH -e myjob_%j.e       # Name of stderr output file(%j expands to jobId)
# SBATCH -N 2                # Total # of nodes
# SBATCH -n 2
# SBATCH -c 4                # Cores per task requested
#SBATCH -p ilk
#SBATCH -t 00:10:00
#SBATCH --mem-per-cpu=1G
#SBATCH -c 4
#########################################
# SBATCH hetjob
# SBATCH -c 4
# SBATCH -p ilk
# SBATCH -t 00:10:00
# SBATCH --mem-per-cpu=2GB
####################################

# SBATCH -N 2                # Total # of nodes
# SBATCH -c 1                # Cores per task requested
# SBATCH -p ilk
# SBATCH -n 1               # Total # of tasks
# SBATCH -t 00:10:00         # Run time (hh:mm:ss) - 10 min
# SBATCH --mem-per-cpu=1G    # Memory per core demandes (24 GB = 3GB * 8 cores)38187
# SBATCH -n 2
# SBATCH -t 00:10:00
# SBATCH --mem=9GB
# SBATCH --mem-per-cpu=2GB

#module load intel impi


conda activate cuda-quantum
module load qmio/hpc gcc/12.3.0 impi/2021.13.0

srun python server.py
#srun --het-group=0 python server.py 0 : --het-group=1 python server.py 1


################ OTRA FORMA DE HACER SRUN ###############
#srun -t 00:10:00 --mem-per-cpu=1GB ./prog1 : -t 00:05:00 --mem-per-cpu=2GB python ejemplo.py


#echo "done"                 # Write this message on the output file when finnished

