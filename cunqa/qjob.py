"""
    Contains objects that define and manage quantum emulation jobs.

    The core of this module is the class :py:class:`~cunqa.qjob.QJob`. These objects are created when a quantum job
    is sent to a virtual QPU, as a return of the :py:meth:`~cunqa.qpu.QPU.run` method:

        >>> qpu.run(circuit)
        <cunqa.qjob.QJob object at XXXXXXXX>
        
    Once it is created, the circuit is being simulated at the virtual QPU.
    :py:class:`QJob` is the bridge between sending a circuit with instructions and recieving the results.
    Because of this, usually one wants to save this output in a variable:

        >>> qjob = qpu.run(circuit)

    Another functionality described in the submodule is the function :py:func:`~gather`, 
    which recieves a list of :py:class:`~QJob` objects and returns their results as :py:class:`~cunqa.result.Result`
    objects.

        >>> qjob_1 = qpu_1.run(circuit_1)
        >>> qjob_2 = qpu_2.run(circuit_2)
        >>> results = gather([qjob_1, qjob_2])
    
    For further information on sending and gathering quantum jobs, chekout the `Examples Galery <https://cesga-quantum-spain.github.io/cunqa/examples_gallery.html>`_.


    """

from typing import  Union, Any
import json
from typing import  Optional, Union, Any
from qiskit import QuantumCircuit
from qiskit.qasm2.exceptions import QASM2Error
from qiskit.exceptions import QiskitError

from cunqa.circuit import qc_to_json,CunqaCircuit, _registers_dict
from cunqa.logger import logger
from cunqa.backend import Backend
from cunqa.result import Result
from cunqa.qclient import QClient, FutureWrapper


class QJobError(Exception):
    """Exception for error during job submission to virtual QPUs."""
    pass

class QJob:
    """
    Class to handle jobs sent to virtual QPUs.

    A :py:class:`QJob` object is created as the output of the :py:meth:`~cunqa.qpu.QPU.run` method.
    The quantum job not only contains the circuit to be simulated, but also simulation instructions and information of the virtual QPU to which the job is sent.

    Hadling QJobs
    =============


    Upgrading parameters from QJobs
    =============================== 


    """
    _backend: 'Backend' 
    qclient: 'QClient' #: Client linked to the server that listens at the virtual QPU.
    _updated: bool
    _future: 'FutureWrapper' 
    _result: Optional['Result']
    _circuit_id: str 
    _sending_to: "list[str]"
    _is_dynamic: bool
    _has_cc:bool

    def __init__(self, qclient: 'QClient', backend: 'Backend', circuit: Union[dict, 'CunqaCircuit', 'QuantumCircuit'], **run_parameters: Any):
        """
        Initializes the :py:class:~`QJob` class.

        Possible instructions to add as `**run_parameters` can be: *shots*, *method*, *parameter_binds*, *meas_level*, ...
        For further information, check :py:meth:`~cunqa.qpu.QPU.run` method.

        .. warning::

            At this point, *circuit* is asumed to be translated into the native gates of the *backend*.
            Otherwise, simulation will fail and an error will be returned as the result.
            For further details, checkout :py:mod:`~cunqa.transpile`.

        Args:
            qclient (QClient): client linked to the server that listens at the virtual QPU.

            backend (~cunqa.backend.Backend): gathers necessary information about the simulator.

            circ (qiskit.QuantumCircuit | dict | ~cunqa.circuit.CunqaCircuit): circuit to be run.

            **run_parameters : any other simulation instructions.

        """

        self._backend: 'Backend' = backend
        self._qclient: 'QClient' = qclient
        self._updated: bool = False
        self._future: 'FutureWrapper' = None
        self._result: Optional['Result'] = None
        self._circuit_id: str = ""

        self._convert_circuit(circuit)
        self._configure(**run_parameters)
        logger.debug("Qjob configured")

    @property
    def result(self) -> 'Result':
        """
        Result of the job.
        If no error occured during simulation, a :py:class:`~cunqa.result.Result` object is retured.
        Otherwise, :py:class:`~cunqa.result.ResultError` will be raised.

        .. note::
            Since to obtain the result the simulation has to be finished, this method is a blocking call,
            which means that the program will be blocked until the :py:class:`QClient` has recieved from
            the corresponding server the outcome of the job.
            The result is not sent from the server to the :py:class:`QClient` until this method is called.

        """
        try:
            if self._future is not None and self._future.valid():
                if self._result is not None:
                    if not self._updated: # if the result was already obtained, we only call the server if an update was done
                        res = self._future.get()
                        self._result = Result(json.loads(res), circ_id=self._circuit_id, registers=self._cregisters)
                        self._updated = True
                    else:
                        pass
                else:
                    res = self._future.get()
                    self._result = Result(json.loads(res), self._circuit_id, registers=self._cregisters)
                    self._updated = True
            else:
                logger.debug(f"self._future is None or non-valid, None is returned.")
        except Exception as error:
                logger.error(f"Error while reading the results {error}")
                raise SystemExit # User's level
        
        if self._backend.simulator == "CunqaSimulator" and self.num_clbits != self.num_qubits:
            logger.warning(f"Be aware that for CunqaSimualtor, number of clbits is required to be equal than the number of qubits of the circuit. Classical bits can appear to be rewritten.")

        return self._result

    @property
    def time_taken(self) -> str:
        """
        Time that the job took.
        """

        if self._future is not None and self._future.valid():
            if self._result is not None:
                try:
                    return self._result.time_taken
                except AttributeError:
                    logger.warning("Time taken not available.")
                    return ""
            else:
                logger.error(f"QJob not finished [{QJobError.__name__}].")
                raise SystemExit # User's level
        else:
            logger.error(f"No QJob submited [{QJobError.__name__}].")
            raise SystemExit # User's level

    def submit(self) -> None:
        """
        Asynchronous method to submit a job to the corresponding :py:class:`QClient`.

        .. note::
            Differently from :py:meth:`~QJob.result`, this is a non-blocking call.
            Once a job is summited, there is no wait, the python program continues at the same time that
            the corresponding server recieves and simualtes the circuit.
        """
        if self._future is not None:
            logger.warning("QJob has already been submitted.")
        else:
            try:
                self._future = self._qclient.send_circuit(self._execution_config)
                logger.debug("Circuit was sent.")
            except Exception as error:
                logger.error(f"Some error occured when submitting the job [{type(error).__name__}].")
                raise QJobError # I capture the error in QPU.run() when creating the job
            
    def upgrade_parameters(self, parameters: "list[float | int]") -> None:
        """
        Method to upgrade the parameters in a previously submitted parametric circuit.
        By this call, firstly the result is called from the server and afterwards the new parameters are sent.

        Args:
            parameters (list[float | int]): list of parameters to assign to the parametrized circuit.
        """

        if self._result is None:
            self._future.get()

        if isinstance(parameters, list):

            if all(isinstance(param, (int, float)) for param in parameters):  # Check if all elements are real numbers
                message = """{{"params":{} }}""".format(parameters).replace("'", '"')

            else:
                logger.error(f"Parameters must be real numbers [{ValueError.__name__}].")
                raise SystemExit # User's level
        
            try:
                #logger.debug(f"Sending new parameters to circuit {self._circuit_id}.")
                self._future = self._qclient.send_parameters(message)

            except Exception as error:
                logger.error(f"Some error occured when sending the new parameters to circuit {self._circuit_id} [{type(error).__name__}].")
                raise SystemExit # User's level
        else:
            logger.error(f"Ivalid parameter type, list was expected but {type(parameters)} was given. [{TypeError.__name__}].")
            raise SystemExit # User's level            
        
        self._updated = False # We indicate that new results will come, in order to call server

    def _convert_circuit(self, circuit: Union[str, dict, 'CunqaCircuit', 'QuantumCircuit']) -> None:
        try:
            if isinstance(circuit, dict):

                logger.debug("A circuit dict was provided.")

                self.num_qubits = circuit["num_qubits"]
                self.num_clbits = circuit["num_clbits"]
                self._cregisters = circuit["classical_registers"]
                if "sending_to" in circuit:
                    self._sending_to = circuit["sending_to"]
                else:
                    self._sending_to = []
                if "has_cc" in circuit:
                    self._has_cc = circuit["has_cc"]
                    self._is_dynamic = True
                elif "is_dynamic" in  circuit:
                    self._is_dynamic = circuit["is_dynamic"]
                else:
                    self._is_dynamic = False
                    self._has_cc = False

                logger.debug("Translation to dict not necessary...")

                # might explode for handmade dicts not design for ditributed execution
                self._circuit_id = circuit["id"]
                instructions = circuit['instructions']
            

            elif isinstance(circuit, CunqaCircuit):

                logger.debug("A CunqaCircuit was provided.")

                self.num_qubits = circuit.num_qubits
                self.num_clbits = circuit.num_clbits
                self._cregisters = circuit.classical_regs
                self._circuit_id = circuit._id
                self._sending_to = circuit.sending_to
                self._is_dynamic = circuit.is_dynamic
                self._has_cc = circuit.has_cc
                
                logger.debug("Translating to dict from CunqaCircuit...")

                instructions = circuit.instructions


            elif isinstance(circuit, QuantumCircuit):

                logger.debug("A QuantumCircuit was provided.")

                self.num_qubits = circuit.num_qubits
                self.num_clbits = sum([c.size for c in circuit.cregs])
                self._cregisters = _registers_dict(circuit)[1]
                self._sending_to = []

                logger.debug("Translating to dict from QuantumCircuit...")

                circuit_json, is_dynamic = qc_to_json(circuit)
                instructions = circuit_json['instructions']
                self._is_dynamic = is_dynamic
                self._has_cc = False

            elif isinstance(circuit, str):

                logger.debug("A QASM2 circuit was provided.")

                qc_from_qasm = QuantumCircuit.from_qasm_str(circuit)

                self.num_qubits = qc_from_qasm.num_qubits
                self._cregisters = _registers_dict(qc_from_qasm)[1]
                self.num_clbits = sum(len(k) for k in self._cregisters.values())
                self._cregisters = _registers_dict(qc_from_qasm)[1]
                # TODO: ¿self.circuit_id?
                self._sending_to = []

                logger.debug("Translating to dict from QASM2 string...")

                circuit_json, is_dynamic = qc_to_json(circuit)
                instructions = circuit_json['instructions']
                self._is_dynamic = is_dynamic
                self._has_cc = False

            else:
                logger.error(f"Circuit must be dict, <class 'cunqa.circuit.CunqaCircuit'> or QASM2 str, but {type(circuit)} was provided [{TypeError.__name__}].")
                raise QJobError # I capture the error in QPU.run() when creating the job

            self._circuit = instructions            
            
        except KeyError as error:
            logger.error(f"Format of the circuit dict not correct, couldn't find 'num_clbits', 'classical_registers' or 'instructions' [{type(error).__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job

        except QASM2Error as error:
            logger.error(f"Error while translating to QASM2 [{type(error).__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job
    
        except QiskitError as error:
            logger.error(f"Format of the circuit not correct  [{type(error).__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job
    
        except Exception as error:
            logger.error(f"Some error occured with the circuit dict provided [{type(error).__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job


    def _configure(self, **run_parameters: Any) -> None:
        # configuration
        try:
            # config dict
            run_config = {
                "shots": 1024, 
                "method":"statevector", 
                "num_clbits": self.num_clbits, 
                "num_qubits": self.num_qubits, 
                "seed": 123123}

            if (run_parameters == None) or (len(run_parameters) == 0):
                logger.debug("No run parameters provided, default were set.")
                pass
            elif (type(run_parameters) == dict): 
                for k,v in run_parameters.items():
                    run_config[k] = v
            else:
                logger.warning("Error when reading `run_parameters`, default were set.")
            
            logger.debug("Before exec_config")
            exec_config = {
                "id": self._circuit_id,
                "config": run_config, 
                "instructions": self._circuit,
                "sending_to": self._sending_to,
                "is_dynamic": self._is_dynamic,
                "has_cc": self._has_cc
            }
            self._execution_config = json.dumps(exec_config)

            logger.debug("QJob created.")
            logger.debug(self._execution_config)

        except KeyError as error:
            logger.error(f"Format of the cirucit not correct, couldn't find 'instructions' [{type(error).__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job
        
        except Exception as error:
            logger.error(f"Some error occured when generating configuration for the simulation [{type(error).__name__}].")
            raise QJobError # I capture the error in QPU.run() when creating the job
        


def gather(qjobs: Union[QJob, "list['QJob']"]) -> Union[Result, "list['Result']"]:
    """
        Function to get result of several QJob objects, it also takes one QJob object.

        Args:
            qjobs (list of QJob objects or QJob object)

        Return:
            Result or list of results.
    """
    if isinstance(qjobs, list):
        if all([isinstance(q, QJob) for q in qjobs]):
            return [q.result for q in qjobs]
        else:
            logger.error(f"Objects of the list must be <class 'qjob.QJob'> [{TypeError.__name__}].")
            raise SystemExit # User's level
            
    elif isinstance(qjobs, QJob):
        return qjobs.result

    else:
        logger.error(f"qjobs must be <class 'qjob.QJob'> or list, but {type(qjobs)} was provided [{TypeError.__name__}].")
        raise SystemError # User's level
