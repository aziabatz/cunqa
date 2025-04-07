import os
import sys
import json
import subprocess
from subprocess import run
from qiskit import QuantumCircuit
from qiskit.circuit.library import QFT
from cunqa.circuit import qc_to_json
from cunqa.qpu import getQPUs

# Adding pyhton folder path to detect modules
INSTALL_PATH = os.getenv("INSTALL_PATH")
sys.path.insert(0, INSTALL_PATH)

class QRaiseError(Exception):
    """Exception for errors during qraise slurm command"""
    pass

def create_QPU(how_many, time, flags = ''):
    """
    Raises a QPU and returns its job_id

    Args
    -----------
    how_many (int): number of QPUs to be raised
    time (str, format: 'D-HH:MM:SS'): maximun time that the classical resources will be reserved for the QPU
    flags (str): any other flag you want to apply. It's empty by default

    """
    
    # assert type(how_many) == int,  f'Number must be int, but {type(how_many)} was provided' 
    # time.replace(" ", "") #remove all whitespaces in time
    # assert all([type(time) == str, time[0].isdecimal(), time[1].isdecimal(), time[2] == ':', time[3].isdecimal(), time[4].isdecimal(),  time[2] == ':', time[6].isdecimal(), time[7].isdecimal(), len(time) == 8]), 'Incorrect time format, it should be D-HH:MM:SS'

    try:
        cmd = ["qraise", "-n", str(how_many), '-t', str(time), str(flags)]
        # output = run(cmd, capture_output=True).stdout #run the command on terminal and capture ist output on the variable 'output'

        # job_id = ''.join(e for e in str(output) if e.isdecimal()) #checks the output on the console (looks like 'Submitted batch job 136285') and selects the number
        # return job_id
    
    except Exception as error:
        raise QRaiseError(f"Unable to raise requested QPUs [{error}]")

def qdrop(*qpus):
    """
    Drops the QPU families corresponding to the the entered QPU objects. By default, all raised QPUs will be dropped

    Args
    --------
    qpus (tuple(<class cunqa.qpu.QPU>)): list of QPUs to drop. All QPUs that share a qraise will these will drop
    """
    
    if len( qpus ) == 0:
        qpus = ['--all'] #if no QPU is provided we drop all QPU slurm jobs

    cmd = ['qdrop']
    for qpu in qpus: # Right now if two qpus have the same job_id it will be printed twice in the command. This is not an issue. The design is lazy as I think we'll go for a diff approach and I wanna minimize useless work
        job_id = qpu.port[0:6] #Port looks like 136838_0, where the first part is the job id and the second part is the QPU id. We discard the QPU id
        cmd.append(str(job_id))
    run(cmd) #run 'qdrop slurm_job_id_1 slurm_job_id_2 etc' on terminal
    return