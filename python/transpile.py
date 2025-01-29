from backend import Backend
from qiskit import QuantumCircuit
from circuit import from_json_to_qc, qc_to_json
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit.providers.models import BackendConfiguration
from qiskit.providers.backend_compat import convert_to_target


def transpiler(circuit, backend, opt_level = 1):
    
    # transformo el circuito a QuantumCircuit
    if isinstance(circuit, QuantumCircuit):
        circuit = circuit
    elif isinstance(circuit, dict):
        try:
            circuit = from_json_to_qc(circuit)
        except:
            raise KeyError("Circuit json not correct, requiered keys must be: 'instructions', 'num_qubits', 'num_clbits', 'quantum_resgisters' and 'classical_registers'.")
    else:
        raise TypeError("Circuit must be <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'> or dict.")
    

    # Limpio bien el diccionario de configuración para generar el target apropiado

    if isinstance(backend, Backend):
        configuration = backend.__dict__
    else:
        raise TypeError("backend must be <class 'python.backend.Backend'>")
    
    
    
    args = {
        "backend_name": configuration["name"],
        "backend_version": configuration["version"],
        "n_qubits": configuration["n_qubits"],
        "basis_gates": configuration["basis_gates"],
        "gates":[],# TODO: comprobar que esto no peta y que funciona adecuadamente
        "local":False,
        "simulator":configuration["is_simulator"],
        "conditional":configuration["conditional"],
        "open_pulse":False,# TODO: lo ponemos así porque Aer no soporta OpenPulse, habría que modificarlo en caso de usar un backend que lo soporte.
        "memory":configuration["memory"],
        "max_shots":configuration["max_shots"],
        "coupling_map":[]#configuration["coupling_map"]
    }

    backend_configuration = BackendConfiguration(**args)

    target =  convert_to_target(backend_configuration)
    pm = generate_preset_pass_manager(optimization_level = opt_level, target = target)

    circuit_transpiled = pm.run(circuit)

    return qc_to_json(circuit_transpiled)