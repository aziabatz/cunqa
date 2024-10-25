#!/bin/bash

# Primera tarea
#SBATCH --ntasks=3
#SBATCH --cpus-per-task=4
#SBATCH --mem=8G
#SBATCH -t 00:01:00

#ml load gcc boost nlohmann_json/3.11.3 impi/2021.13.0
#g++ protoserver.cpp -o server -lboost_system -lpthread

# Ejecutar tareas
srun prueba