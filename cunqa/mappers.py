from cunqa.logger import logger
from cunqa.qjob import gather
from qiskit import QuantumCircuit
from qiskit.exceptions import QiskitError
from qiskit.qasm2 import QASM2Error
from cunqa.circuit import from_json_to_qc

class QJobMapper:
    """
    Class to map the function `QJob.upgrade_parameters()` to a list of QJobs.
    """
    def __init__(self, qjobs):
        """
        Initializes the QJobMapper class.

        Args:
        ---------
        qjobs (list[<class 'qjob.QJob'>]): list of QJobs to be mapped.

        """
        self.qjobs = qjobs
        self.i = 0  # starting point
        logger.debug(f"QJobMapper initialized with {len(qjobs)} QJobs.")

    def __call__(self, func, population):
        """
        Callable method to map the function to the given QJobs.

        Args:
        ---------
        func (func): function to be mapped to the QJobs. It must take as argument the an object <class 'qjob.Result'>.

        population (list[list]): population of vectors to be mapped to the QJobs.

        Return:
        ---------
        List of results of the function applied to the QJobs for the given population.
        """

        qjobs = []

        for params in population:
            qjob = self.qjobs[self.i % len(self.qjobs)]

            logger.debug(f"Uptading params for QJob {qjob} in QPU {qjob._QPU.id}...")
            qjobs.append(qjob.upgrade_parameters(params.tolist()))

            self.i += 1

        logger.debug("About to gather results ...")
        results = gather(qjobs)
        self.i = 0
        return [func(result) for result in results]


class QPUCircuitMapper:
    """
    Class to map the function `qpu.QPU.run()` to a list of QPUs.
    """
    def __init__(self, qpus, circuit, transpile = False, initial_layout = None, **run_parameters):
        """
        Initializes the QPUCircuitMapper class.

        Args:
        ---------
        qpus (list[<class 'qpu.QPU'>]): list of QPU objects.

        circuit (json dict, <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'> or QASM2 str): circuit to be run in the QPUs.

        transpile (bool): if True, transpilation will be done with respect to the backend of the given QPU. Default is set to False.

        initial_layout (list[int]): Initial position of virtual qubits on physical qubits for transpilation.

        **run_parameters : any other simulation instructions.

        """
        self.qpus = qpus

        try:
            if isinstance(circuit, dict):
                self.circuit = from_json_to_qc(circuit)

            elif isinstance(circuit, QuantumCircuit):
                self.circuit = circuit
                
            elif isinstance(circuit, str):
            
                circuit = QuantumCircuit.from_qasm_str(circuit)
            
                self.circuit = circuit

            else:
                logger.error(f"Circuit must be dict, <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'> or QASM2 str, but {type(circuit)} was provided [{TypeError.__name__}].")
                raise SystemExit # User's level
            
        except QASM2Error as error:
            logger.error(f"Error with QASM2 string, please check that the sintex is correct [{type(error).__name__}]: {error}.")
            raise SystemExit # User's level
        
        except  QiskitError as error:
            logger.error(f"Error with QuantumCircuit [{type(error).__name__}]: {error}.")
            raise SystemExit # User's level
        
        except Exception as error:
            logger.error(f"Some error occurred with the circuit format [{type(error).__name__}]: {error}")
            raise SystemExit # User's level
            
        self.i = 0  # starting point
        self.transpile = transpile
        self.initial_layout = initial_layout
        self.run_parameters = run_parameters
        
        logger.debug(f"QPUMapper initialized with {len(qpus)} QPUs.")

    def __call__(self, func, population):
        """
        Callable method to map the function to the QPUs.

        Args:
        ---------
        func (func): function to be mapped to the QPUs. It must take as argument the an object <class 'qjob.Result'>.

        params (list[list]): population of vectors to be mapped to the circuits sent to the QPUs.

        Return:
        ---------
        List of the results of the function applied to the output of the circuits sent to the QPUs.
        """

        qjobs = []

        for params in population:
            qpu = self.qpus[self.i % len(self.qpus)]
            circuit_assembled = self.circuit.assign_parameters(params)

            logger.debug(f"Sending QJob to QPU {qpu.id}...")
            qjobs.append(qpu.run(circuit_assembled, transpile = self.transpile, initial_layout = self.initial_layout, **self.run_parameters))
            
            self.i += 1
        
        logger.debug("About to gather results ...")
        results = gather(qjobs)

        self.i = 0

        return [func(result) for result in results]