import os
import time

#print("NUMBER OF TASKS: ", os.getenv("SLURM_NTASKS"))
#print("CPUS PER TASK: ", os.getenv("SLURM_CPUS_PER_TASK"))
print("\t Soy el PYTHON")

time.sleep(5)

print(
    "\t I am task: ",
    os.getenv("SLURM_PROCID"),
    "NUMBER OF TASKS: ",
    os.getenv("SLURM_NTASKS"),
    "CPUS PER TASK: ",
    os.getenv("SLURM_CPUS_PER_TASK"),
    "SLURM_STEP_RESV_PORTS",
    os.getenv("SLURM_STEP_RESV_PORTS"),
    "SLURM_LOCALID",
    os.getenv("SLURM_LOCALID")
)

os.system("touch TAREA"+os.getenv("SLURM_PROCID")+".txt")

