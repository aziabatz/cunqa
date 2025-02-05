from concurrent.futures import ThreadPoolExecutor
import os
import sys
import pickle, json
import time
from qiskit import QuantumCircuit
from circuit import qc_to_json
from transpile import transpiler
import qpu

# path to access c++ files
installation_path = os.getenv("INSTALL_PATH")
sys.path.append(installation_path)

from cunqa.qclient import QClient

# importing logger
from logger import logger


class QJobError(Exception):
    """Exception for error during job submission to QPUs."""
    pass


def _divide(string, lengths):

    """
    Divides a string of bits in groups of given lenghts separated by spaces.

    Args:
    --------
    string (str): srting that we want to divide.

    lengths (list[int]): lenghts of the resulting strings in which the original one is divided.

    Return:
    --------
    A new string in which the resulting groups are separated by spaces.

    """

    parts = []
    init = 0
    try:
        for length in lengths:
            parts.append(string[init:init + length])
            init += length
        return ' '.join(parts)
    
    except Exception as error:
        logger.error(f"Something failed with division of string [{error.__name__}].")
        raise SystemExit # User's level



class Result():
    """
    Class to describe the result of an experiment.
    """

    def __init__(self, result, registers = None):
        """
        Initializes the Result class.

        Args:
        -----------
        result (dict): dictionary given as the result of the simulation.

        registers (dict): in case the circuit has more than one classical register, dictionary for the lengths of the classical registers must be provided.
        """

        if type(result) == dict:
            self.result = result
        elif result is None:
            logger.error(f"Something failed at server, result is {None} [{ValueError.__name__}].")
            raise ValueError
        else:
            logger.error(f"result must be dict, but {type(result)} was provided [{TypeError.__name__}].")
            raise TypeError # I capture this error in QJob.result() when creating the object.
        
        if len(result) == 0:
            logger.error(f"Results dictionary is empty, some error occured [{ValueError.__name__}].")
            raise ValueError # I capture this error in QJob.result() when creating the object.
        else:
            counts = None
            for k,v in result.items():
                if k == "metadata":
                    for i, m in v.items():
                        setattr(self, i, m)
                elif k == "results":
                    for i, m in v[0].items():
                        if i == "data":
                            counts = m["counts"]
                        elif i == "metadata":
                            for j, w in m.items():
                                setattr(self,j,w)
                        else:
                            setattr(self, i, m)
                else:
                    setattr(self, k, v)

            self.counts = {}
            if counts:
                for j,w in counts.items():
                    if registers is None:
                        self.counts[format( int(j, 16), '0'+str(self.num_clbits)+'b' )]= w
                    elif isinstance(registers, dict):
                        lengths = []
                        for v in registers.values():
                            lengths.append(len(v))
                        self.counts[_divide(format( int(j, 16), '0'+str(self.num_clbits)+'b' ), lengths)]= w
            else:
                logger.error(f"Some error occured with results file, no `counts` found. Check avaliability of the QPUs [{KeyError.__name__}].")
                raise KeyError # I capture this error in QJob.result() when creating the object.
            
        logger.debug("Results correctly loaded.")
        
    def get_dict(self):
        """
        Class method to obtain a dictionary with the class variables.

        Return:
        -----------
        Dictionary with the class variables, actually the result input but with some format changes.

        """
        return self.result

    def get_counts(self):
        """
        Class method to obtain the information for the counts result for the given experiment.

        Return:
        -----------
        Dictionary in which the keys are the bit strings and the values number of times that they were measured.

        """
        return self.counts



class QJob():
    """
    Class to handle jobs sent to the simulator.
    """

    def __init__(self, QPU, circ, transpile, initial_layout = None, **run_parameters):
        """
        Initializes the QJob class.

        It is important to note that  if `transpilation` is set False, we asume user has already done the transpilation, otherwise some errors during the simulation
        can occur, for example if the QPU has a noise model with error associated to specific gates, if the circuit is not transpiled errors might not appear.

        If `transpile` is False and `initial_layout` is provided, it will be ignored.

        Possible instructions to add as `**run_parameters` can be: shots, method, parameter_binds, meas_level, ...


        Args:
        -----------
        QPU (<class 'qpu.QPU'>): QPU object that represents the virtual QPU to which the job is going to be sent.

        circ (json dict or <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'>): circuit to be run.

        transpile (bool): if True, transpilation will be done with respect to the backend of the given QPU. Default is set to False.

        initial_layout (list[int]):  initial position of virtual qubits on physical qubits for transpilation, lenght must be equal to the number of qubits in the circuit.

        **run_parameters : any other simulation instructions.

        """
        if isinstance(QPU, qpu.QPU):
            self._QPU = QPU
        else:
            logger.error(f"QPU must be <class 'qpu.QPU'>, but {type(QPU)} was provided [{TypeError.__name__}].")
            raise QJobError # I capture this error in QPU.run() when creating the job
        
        self._future = None
        self._result = None


        if isinstance(circ, QuantumCircuit) or isinstance(circ, dict):

            if transpile:
                try:
                    circt = transpiler( circ, QPU.backend, initial_layout = initial_layout )
                    logger.debug("Transpilation done.")
                except Exception as error:
                    logger.error(f"Transpilation failed [{error.__name__}].")
                    raise QJobError # I capture the error in QPU.run() when creating the job
                
            else:
                if initial_layout is not None:
                    logger.warning("Transpilation was not done, initial_layout provided was ignored. If you want to map the circuit to the given qubits of the backend you must set transpile=True and provide the list of qubits which lenght has to be equal to the number of qubits of the circuit.")
                circt = circ
                logger.warning("No transpilation was done, errors might occur if any gate or instruction is not supported by the simulator.")


            if isinstance(circt, dict):
                circuit = circt
                
            elif isinstance(circt, QuantumCircuit):
                circuit = qc_to_json(circt)

            
            self._circuit = circuit

        else:
            logger.error(f"Circuit must be dict or <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'>, but {type(circ)} was provided [{TypeError.__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job
    

        try:
            # config dict
            run_config = {"shots":1024, "method":"statevector", "memory_slots":circuit["num_clbits"], "seed":188}

            if run_parameters == None:
                logger.debug("No run parameters provided, default were set.")
                pass
            elif (type(run_parameters) == dict) or (len(run_parameters) == 0):
                for k,v in run_parameters.items():
                    run_config[k] = v
            else:
                logger.warning("Error when reading `run_parameters`, default were set.")
            
            # instructions dict
            instructions = circuit['instructions']

            self._execution_config = """ {{"config":{}, "instructions":{} }}""".format(run_config, instructions).replace("'", '"')

        
        except KeyError as error:
            logger.error(f"Format of the cirucit not correct, couldn't find 'instructions' [{error.__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job
        
        except Exception as error:
            logger.error(f"Some error occured when generating configuration for the simulation [{error.__name__}]")
            raise QJobError # I capture the error in QPU.run() when creating the job


    def submit(self):
        """
        Asynchronous method to submit a job to the corresponding QClient.
        """
        if self._future is not None:
            logger.warning("QJob has already been submitted.")
        else:
            try:
                self._future = self._QPU._qclient.send_circuit(self._execution_config)
            except Exception as error:
                logger.error(f"Some error occured when submitting the job [{error.__name__}].")
                raise QJobError # I capture the error in QPU.run() when creating the job

    def result(self):
        """
        Synchronous method to obtain the result of the job. Note that this call depends on the job being finished, therefore is bloking.
        """
        if (self._future is not None) and (self._future.valid()):
            if self._result is None:
                try:
                    self._result = Result(json.loads(self._future.get()), registers=self._circuit['classical_registers'])
                except Exception as error:
                    logger.error(f"Error while creating Results object [{error.__name__}]")
                    raise SystemExit # User's level

        return self._result

    def time_taken(self):
        """
        Method to obtain the time that the job took. It can also be obtained by `QJob._result.time_taken`.
        """

        if self._future is not None and self._future.valid():
            if self._result is not None:
                return self._result.get_dict()["results"][0]["time_taken"]
            else:
                logger.error(f"QJob not finished [{QJobError.__name__}].")
                raise SystemExit # User's level
        else:
            logger.error(f"No QJob submited [{QJobError.__name__}].")
            raise SystemExit # User's level
        


def gather(qjobs):
    """
        Function to get result of several QJob objects, it also takes one QJob object.

        Args:
        ------
        qjobs (list of QJob objects or QJob object)

        Return:
        -------
        Result or list of results.
    """
    if isinstance(qjobs, list):
        if all([isinstance(q, QJob) for q in qjobs]):
            return [q.result() for q in qjobs]
        else:
            logger.error(f"Objects of the list must be <class 'qjob.QJob'> [{TypeError.__name__}].")
            raise SystemExit # User's level
            
    elif isinstance(qjobs, QJob):
        return qjobs.result()

    else:
        logger.error(f"qjobs must be <class 'qjob.QJob'> or list, but {type(qjobs)} was provided [{TypeError.__name__}].")
        raise SystemError # User's level

      
        
               
        
