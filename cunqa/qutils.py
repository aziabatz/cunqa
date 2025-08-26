""" Holds functions that manage virtual QPUs or provide information about them."""
import os
import sys
import time
from typing import Union, Optional
from subprocess import run
from json import load
import json
from cunqa.qclient import QClient
from cunqa.backend import Backend
from cunqa.logger import logger
from cunqa.qpu import QPU

# Adding python folder path to detect modules
sys.path.append(os.getenv("HOME"))

STORE: Optional[str] = os.getenv("STORE")
if STORE is not None:
    INFO_PATH = STORE + "/.cunqa/qpus.json"
else:
    logger.error(f"Cannot find $STORE enviroment variable.")
    raise SystemExit

class QRaiseError(Exception):
    """Exception for errors during qraise slurm command."""
    pass

def are_QPUs_raised(family: Optional[str] = None) -> bool:
    """
    Checks availability of QPUs, filtering by `family` if desired.

    Args:
        family (str): name of the family of the QPUs whose availability is checked.
    
    Returns:
        ``True`` if those are deployed, ``False`` if not.
    """
    last_modification = os.stat(INFO_PATH).st_mtime 
    while True:
        if last_modification != os.stat(INFO_PATH).st_mtime: 
            last_modification = os.stat(INFO_PATH).st_mtime
            if family == None:
                return True
            else:
                with open(INFO_PATH, "r") as qpus_json:
                    data = json.load(qpus_json)
                    for value in data.values():
                        if value["family"] == family:
                            return True
                        
def qraise(n, t, *, 
           classical_comm = False, 
           quantum_comm = False,  
           simulator = None, 
           backend = None, 
           fakeqmio = False, 
           family = None, 
           cloud = True, 
           cores = None, 
           mem_per_qpu = None, 
           n_nodes = None, 
           node_list = None, 
           qpus_per_node= None) -> Union[tuple, str]:
    """
    Raises a virtual QPU and returns the job id associated to its `SLURM <https://slurm.schedmd.com/documentation.html>`_ job.

    This function allows to raise QPUs from the python API, what can also be done at terminal by ``qraise`` command.

    Args:
        n (int): number of virtual QPUs to be raised in the job.

        t (str): maximun time that the classical resources will be reserved for the job. Format: 'D-HH:MM:SS'.

        classical_comm (bool): if ``True``, virtual QPUs will allow classical communications.

        quantum_comm (bool): if ``True``, virtual QPUs will allow quantum communications.

        simulator (str): name of the desired simulator to use. Default is `Aer <https://github.com/Qiskit/qiskit-aer>`_.

        backend (str): path to a file containing backend information.

        fakeqmio (bool): ``True`` for raising `n` virtual QPUs with FakeQmio backend.

        family (str): name to identify the group of virtual QPUs raised.

        cloud (bool): if ``True``, `cloud` mode is set, otherwise `hpc` mode is set. In `hpc` mode, virtual QPUs can only be accessed from the node in which they are deployed. In `cloud` mode, they can be accessed from other nodes.

        cores (str):  number of cores per virtual QPU, the total for the SLURM job will be `n*cores`.

        mem_per_qpu (str): memory to allocate for each virtual QPU in GB, format to use is  "xG".

        n_nodes (str): number of nodes for the SLURM job.

        node_list (str): list of nodes in which the virtual QPUs will be deployed.

        qpus_per_node (str): sets the number of virtual QPUs deployed on each node.
    
    Returns:
        The SLURM job id of the job deployed. If `family` was provided, a tuple (`family`, `job id`).
    """
    logger.debug("Setting up the requested QPUs...")

    SLURMD_NODENAME = os.getenv("SLURMD_NODENAME")

    if SLURMD_NODENAME == None:
        command = f"qraise -n {n} -t {t}"
    else: 
        logger.warning("Be careful, you are deploying QPUs from an interactive session.")
        HOSTNAME = os.getenv("HOSTNAME")
        command = f"ssh {HOSTNAME} \"ml load qmio/hpc gcc/12.3.0 hpcx-ompi flexiblas/3.3.0 boost cmake/3.27.6 pybind11/2.12.0-python-3.9.9 nlohmann_json/3.11.3 ninja/1.9.0 qiskit/1.2.4-python-3.9.9 && cd bin && ./qraise -n {n} -t {t}"

    try:
        # Add specified flags
        if fakeqmio:
            command = command + " --fakeqmio"
        if classical_comm:
            command = command + " --classical_comm"
        if quantum_comm:
            command = command + " --quantum_comm"
        if simulator is not None:
            command = command + f" --simulator={str(simulator)}"
        if family is not None:
            command = command + f" --family={str(family)}"
        if cloud:
            command = command + " --cloud"
        if cores is not None:
            command = command + f" --cores={str(cores)}"
        if mem_per_qpu is not None:
            command = command + f" --mem_per_qpu={str(mem_per_qpu)}"
        if n_nodes is not None:
            command = command + f" --n_nodes={str(n_nodes)}"
        if node_list is not None:
            command = command + f" --node_list={str(node_list)}"
        if qpus_per_node is not None:
            command = command + f" --qpus_per_node={str(qpus_per_node)}"
        if backend is not None:
            command = command + f" --backend={str(backend)}"

        if SLURMD_NODENAME != None:
            command = command + "\""

        if not os.path.exists(INFO_PATH):
           with open(INFO_PATH, "w") as file:
                file.write("{}")

        output = run(command, shell=True, capture_output=True, text=True).stdout #run the command on terminal and capture its output on the variable 'output'
        logger.info(output)
        job_id = ''.join(e for e in str(output) if e.isdecimal()) #sees the output on the console (looks like 'Submitted sbatch job 136285') and selects the number
        
        cmd_getstate = ["squeue", "-h", "-j", job_id, "-o", "%T"]
        
        i = 0
        while True:
            state = run(cmd_getstate, capture_output=True, text=True, check=True).stdout.strip()
            if state == "RUNNING":
                try:     
                    with open(INFO_PATH, "r") as file:
                        data = json.load(file)
                except json.JSONDecodeError:
                    continue
                count = sum(1 for key in data if key.startswith(job_id))
                if count == n:
                    break
            # We do this to prevent an overload to the Slurm deamon through the 
            if i == 500:
                time.sleep(2)
            else:
                i += 1

        # Wait for QPUs to be raised, so that get_QPUs can be executed inmediately
        print("QPUs ready to work \U00002705")

        return (family, str(job_id)) if family is not None else str(job_id)
    
    except Exception as error:
        raise QRaiseError(f"Unable to raise requested QPUs [{error}].")

def qdrop(*families: Union[tuple[str], str]):
    """
    Drops the virtual QPU families corresponding to the the input family names.
    If no families are provided, all virtual QPUs deployed by the user will be dropped.

    Args:
        families (str): family names of the groups of virtual QPUs to be dropped.
    """
    
    # Building the terminal command to drop the specified families
    cmd = ['qdrop']

    # If no QPU is provided we drop all QPU slurm jobs
    if len( families ) == 0:
        cmd.append('--all') 
    else:
        for family in families:
            if isinstance(family, str):
                cmd.append(family)

            elif isinstance(family, tuple):
                cmd.append(family[1])
            else:
                logger.error(f"Invalid type for qdrop.")
                raise SystemExit
    
 
    run(cmd) #run 'qdrop slurm_jobid_1 slurm_jobid_2 etc' on terminal

def nodes_with_QPUs() -> "list[set]":
    """
    Provides information about the nodes in which virtual QPUs are available.

    Return:
        List of the corresponding node names.
    """
    try:
        with open(INFO_PATH, "r") as f:
            qpus_json = load(f)

        node_names = set()
        for info in qpus_json.values():
            node_names.add(info["net"]["node_name"])

        return list(node_names)

    except Exception as error:
        logger.error(f"Some exception occurred [{type(error).__name__}].")
        raise SystemExit # User's level

def info_QPUs(local: bool = True, node_name: Optional[str] = None) -> "list[dict]":
    """
    Provides information about the virtual QPUs available either in the local node, an specific node or globally.

    If `local` is ``True`` and `node_name` provided is different from the local node, only information at local node will be displayed.
    
    Args:
        local (bool): if ``True`` information at local node is displayed, else all information is displayed.

        node_name (str): filters the displayed information by an specific node.

    Returns:
        A list with :py:class:`dict` objects that display the information of the virtual QPUs.
    
    """

    try:
        with open(INFO_PATH, "r") as f:
            qpus_json = load(f)
            if len(qpus_json) == 0:
                logger.warning(f"No QPUs were found.")
                return [{}]
        
        if node_name is not None:
            targets = [{qpu_id:info} for qpu_id,info in qpus_json.items() if (info["net"].get("node_name") == node_name ) ]
        
        else:
            if local:
                local_node = os.getenv("SLURMD_NODENAME")
                if local_node != None:
                    logger.debug(f"User at node {local_node}.")
                else:
                    logger.debug(f"User at a login node.")
                targets = [{qpu_id:info} for qpu_id,info in qpus_json.items() if (info["net"].get("node_name")==local_node) ]
            else:
                targets =[{qpu_id:info} for qpu_id,info in qpus_json.items()]
        
        info = []
        for t in targets:
            key = list(t.keys())[0]
            info.append({
                "QPU":key,
                "node":t[key]["net"]["node_name"],
                "family":t[key]["family"],
                "backend":{
                    "name":t[key]["backend"]["name"],
                    "simulator":t[key]["backend"]["simulator"],
                    "version":t[key]["backend"]["version"],
                    "description":t[key]["backend"]["description"],
                    "n_qubits":t[key]["backend"]["n_qubits"],
                    "basis_gates":t[key]["backend"]["basis_gates"],
                    "coupling_map":t[key]["backend"]["coupling_map"],
                    "custom_instructiona":t[key]["backend"]["custom_instructions"]
                }
            })
        return info

    except Exception as error:
        logger.error(f"Some exception occurred [{type(error).__name__}].")
        raise error # User's level

def get_QPUs(local: bool = True, family: Optional[str] = None) -> "list['QPU']":
    """
    Returns :py:class:`~cunqa.qpu.QPU` objects corresponding to the virtual QPUs raised by the user.

    Args:
        local (bool): if ``True``, filters by the virtual QPUs available at the local node.
        family (str): filters virtual QPUs by their family name.

    Return:
        List of :py:class:`~cunqa.qpu.QPU` objects.
    
    """

    # access raised QPUs information on qpu.json file
    try:
        with open(INFO_PATH, "r") as f:
            qpus_json = load(f)
            if len(qpus_json) == 0:
                logger.error(f"No QPUs were found.")
                raise SystemExit

    except Exception as error:
        logger.error(f"Some exception occurred [{type(error).__name__}].")
        raise SystemExit # User's level
    
    logger.debug(f"File accessed correctly.")

    # extract selected QPUs from qpu.json information 
    local_node = os.getenv("SLURMD_NODENAME")
    if local_node != None:
        logger.debug(f"User at node {local_node}.")
    else:
        logger.debug(f"User at a login node.")
    if local:
        if family is not None:
            targets = {qpu_id:info for qpu_id, info in qpus_json.items() if (info["net"].get("nodename") == local_node) and (info.get("family") == family)}
        else:
            targets = {qpu_id:info for qpu_id, info in qpus_json.items() if (info["net"].get("nodename") == local_node)}
    else:
        if family is not None:
            targets = {qpu_id:info for qpu_id, info in qpus_json.items() if ((info["net"].get("nodename") == local_node) or (info["net"].get("nodename") != local_node and info["net"].get("mode") == "cloud")) and (info.get("family") == family)}
        else:
            targets = {qpu_id:info for qpu_id, info in qpus_json.items() if (info["net"].get("nodename") == local_node) or (info["net"].get("nodename") != local_node and info["net"].get("mode") == "cloud")}
    
    # create QPU objects from the dictionary information + return them on a list
    qpus = []
    for id, info in targets.items():
        client = QClient()
        endpoint = (info["net"]["ip"], info["net"]["port"])
        qpus.append(QPU(id = id, qclient = client, backend = Backend(info['backend']), family = info["family"], endpoint = endpoint))
    if len(qpus) != 0:
        logger.debug(f"{len(qpus)} QPU objects were created.")
        return qpus
    else:
        logger.error(f"No QPUs where found with the characteristics provided: local={local}, family_name={family}.")
        raise SystemExit
