from backend import Backend
from qiskit import QuantumCircuit
from circuit import from_json_to_qc, qc_to_json
from qiskit.qasm3.exceptions import QASM3Error
from qiskit.exceptions import QiskitError
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit.providers.models import BackendConfiguration
from qiskit.providers.backend_compat import convert_to_target
from qiskit.qasm3 import dumps

from logger import logger


class TranspilerError(Exception):
    """Exception for error during the transpilation of a circuit to a given Backend. """
    pass


def transpiler(qc, backend, opt_level = 1, initial_layout = None):
    """
    Function to transpile a circuit according to a given backend. Cirtcuit must be qiskit QuantumCircuit, dict or QASM3 string. If QASM3 string is provided, function will also return circuit in QASM3.

    Args:
    -----------
    qc (dict, <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'> or str): circuit to be transpiled. 

    backend (<class 'backend.Backend'>): backend which transpilation will be done respect to.

    opt_level (int): optimization level for creating the `qiskit.transpiler.passmanager.StagedPassManager`. Default set to 1.

    initial_layout (list[int]): initial position of virtual qubits on physical qubits for transpilation, lenght must be equal to the number of qubits in the circuit.
    """
    
    if isinstance(qc, QuantumCircuit):
        if initial_layout is not None and len(initial_layout) != qc.num_qubits:
            logger.error(f"initial_layout must be of the size of the circuit: {qc.num_qubits} [{TypeError.__name__}].")
            raise TranspilerError # I capture this error when creating QJob
        else:
            circuit = qc

    elif isinstance(qc, dict):
        logger.debug("In transpilation: circuit is dict.")
        if initial_layout is not None and len(initial_layout) != qc['num_qubits']:
            logger.error(f"initial_layout must be of the size of the circuit: {qc.num_qubits} [{TypeError.__name__}].")
            raise TranspilerError
        else:
            try:
                circuit = from_json_to_qc(qc)
            except Exception as error:
                raise TranspilerError
    
    elif isinstance(qc, str):
        try:
            circuit = QuantumCircuit.from_qasm_str(qc)
        except QASM3Error as error:
            logger.error(f"Error with QASM3 string, please check that the sintex is correct [{type(error).__name__}]: {error}.")
            raise TranspilerError
        except  QiskitError as error:
            logger.error(f"Error with QASM3 string, please check that the logic of the resulting circuit is correct [{type(error).__name__}]: {error}.")
            raise TranspilerError
        except Exception as error:
            logger.error(f"Some error occurred with QASM3 string, please check sintax and logic of the resulting circuit [{type(error).__name__}]: {error}")
            raise TranspilerError

    else:
        logger.error(f"Circuit must be <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'> or dict [{TypeError.__name__}].")
        raise TranspilerError # I capture this error when creating QJob
    

    if isinstance(backend, Backend):
        configuration = backend.__dict__
    else:
        logger.error(f"Transpilation backend must be <class 'python.backend.Backend'> [{TypeError.__name__}].")
        raise TranspilerError # I capture this error when creating QJob
    
    
    try:
        args = {
            "backend_name": configuration["name"],
            "backend_version": configuration["version"],
            "n_qubits":configuration["n_qubits"],
            "basis_gates": configuration["basis_gates"],
            "gates":[], # might not work
            "local":False,
            "simulator":configuration["is_simulator"],
            "conditional":configuration["conditional"],
            "open_pulse":False,# TODO: another simulator distinct from Aer might suppor open pulse.
            "memory":configuration["memory"],
            "max_shots":configuration["max_shots"],
            "coupling_map":configuration["coupling_map"]
        }

        backend_configuration = BackendConfiguration(**args)
        target =  convert_to_target(backend_configuration)
        pm = generate_preset_pass_manager(optimization_level = opt_level, target = target, initial_layout = initial_layout)
        circuit_transpiled = pm.run(circuit)
    
    except KeyError as error:
        logger.error(f"Error in cofiguration of the backend, some keys are missing [{type(error).__name__}].")
        raise TranspilerError
    
    except Exception as error:
        logger.error(f"Some error occurred with configuration of the backend, please check that the formats are correct [{type(error).__name__}].")
        raise TranspilerError # I capture this error when creating QJob


    if isinstance(qc, str):
        try:
            return dumps(circuit)
        except Exception as error:
            logger.error(f"Error during transpilation [{type(error).__name__}].")
            raise TranspilerError
            
    else:
        return qc_to_json(circuit_transpiled)