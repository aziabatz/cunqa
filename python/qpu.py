import os
import sys
import pickle, json

# path para acceder a los paquetes de c++
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

# path para acceder a la informacion sobre las qpus
info_path = os.getenv("INFO_PATH")
if info_path is None:
    STORE = os.getenv("STORE")
    info_path = STORE+"/.api_simulator/qpu.json"
# importamos api en C++
from python.qclient import QClient
# importamos la clase Backend
from backend import Backend
from qjob import QJob, gather
# importamos funciones para transformar circuitos a json
from circuit import qasm2_to_json, qc_to_json


class QPU():
    """
        Class to define a QPU.

        Class methods defined here:
        -----------
    """
    
    def __init__(self, id_=None, qclient=None, backend=None):
        """
        Initializes th QPU class.

        Attributes:
        -----------
        id_ (int): Id assigned to the qpu, simply a int from 0 to n_qpus-1.

        server_endpoint (str): Ip and port route that identifies the server that represents the qpu itself which the worker will
            communicate with.
            
        backend (str): Name of the configuration file for the FakeQmio() class that the server will instanciate. By default is None,
            in that case the server will estabish a default configuration /opt/cesga/qmio/hpc/calibrations/2024_11_04__12_00_02.json.

        """

        # id
        if id_ == None:
            print("QPU id not provided.")
            return
        elif type(id_) == int:
            self.id_ = id_
        else:
            print("QPU id must be int.")
            return

        # qclient
        if qclient == None:
            print("QPU client not assigned.")
            return
        elif isinstance(qclient, QClient):
            self._qclient = qclient
        else:
            print("Invalid QPU client.")
            return

        # backend
        if backend == None:
            print("QPU has no backend info.")
            return
        elif isinstance(backend, Backend):
            self.backend = backend
        else:
            print("backend type must be class Backend.")
            return


    def run(self, circ, transpile = False, **run_parameters):
        """
            Class method to run a circuit in the QPU.

            Args:
            --------
            circ (json): circuit to be run in the QPU.
            **run_parameters : any simulation instructions such as shots, method, parameter_binds, meas_level, init_qubits, ...

            Return:
            --------
            Result object.
        """
        
        qjob = QJob(self, circ, transpile = transpile,**run_parameters)
        qjob.submit()
        return qjob



def getQPUs():
    """
        Global function to get the QPU objects corresponding (......)

        Return:
        ---------
        List of QPU objects.
    
    """

    with open(info_path, "r") as qpus_json:
        dumps = json.load(qpus_json)
        
    if isinstance(dumps, dict):
        qpus = []
        i = 0
        for k, v in dumps.items():
            client = QClient(info_path)
            client.connect(k)
            qpus.append(  QPU(id_ = i, qclient = client, backend = Backend(v['backend'])  )  )
            i+=1
        return qpus
    else:
        print("Incorrect format for "+info_path)



    



        