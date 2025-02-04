def qasm2_to_json(qasm_str):
    """
    Transforms a QASM circuit to json.

    Args:
    ---------
    qc (str): circuit to transform to json.

    Return:
    ---------
    json dict with the circuit information.
    """
    lines = qasm_str.splitlines()
    json_data = {
        "qasm_version": None,
        "includes": [],
        "registers": [],
        "instructions": []
    }
    
    for line in lines:
        line = line.strip()
        if line.startswith("OPENQASM"):
            json_data["qasm_version"] = line.split()[1].replace(";", "")
        elif line.startswith("include"):
            json_data["includes"].append(line.split()[1].replace(";", "").strip('"'))
        elif line.startswith("qreg") or line.startswith("creg"):
            parts = line.split()
            register_type = parts[0]
            name, size = parts[1].split("[")
            size = int(size.replace("];", ""))
            json_data["registers"].append({"type": register_type, "name": name, "size": size})
        else:
            if line:
                parts = line.split()
                operation_name = parts[0]
                if operation_name != "measure":
                    if "," not in parts[1]:
                        qubits = parts[1].lstrip("q[").rstrip("];")
                        json_data["instructions"].append({"name":operation_name, "qubits":[qubits]})

                    #qubits = parts[1].replace(";", "").split(",")
                    else:
                        parts_split = parts[1].rstrip(";").split(",")
                        q_first = parts_split[0].split("[")[1].rstrip("]")
                        print(parts_split)
                        q_second = parts_split[1].split("[")[1].rstrip("]")
                        json_data["instructions"].append({"name":operation_name, "qubits":[q_first, q_second]})
                else:
                    qubits = parts[1].split("[")[1].rstrip("]")
                    memory_aux = parts[3].rstrip(";")
                    if "meas" in memory_aux:
                        memory = memory_aux.lstrip("meas[").rstrip("]")
                    else:
                        memory = memory_aux.lstrip("c[").rstrip("]")
                    json_data["instructions"].append({"name":operation_name, "qubits":[qubits], "memory":memory})

    return json_data

def qc_to_json(qc):
    """
    Transforms a QuantumCircuit to json.

    Args:
    ---------
    qc (QuantumCircuit): circuit to transform to json.

    Return:
    ---------
    json dict with the circuit information.
    """
    quantum_registers = {}
    for qr in qc.qregs:
        quantum_registers[qr.name] = qr.size

    countsq = []

    valuesq = list(quantum_registers.values())

    for i, v in enumerate(valuesq):
        if i == 0:
            countsq.append(list(range(0, v)))
        else:
            countsq.append(list(range(sum(valuesq[:i]), sum(valuesq[:i])+v)))

    for i,k in enumerate(quantum_registers.keys()):
        quantum_registers[k] = countsq[i]


    classical_registers = {}
    for cr in qc.cregs:
        classical_registers[cr.name] = cr.size

    counts = []

    values = list(classical_registers.values())

    for i, v in enumerate(values):
        if i == 0:
            counts.append(list(range(0, v)))
        else:
            counts.append(list(range(sum(values[:i]), sum(values[:i])+v)))

    for i,k in enumerate(classical_registers.keys()):
        classical_registers[k] = counts[i]
    
    json_data = {
        "instructions":[],
        "num_qubits":sum([q.size for q in qc.qregs]),
        "num_clbits": sum([c.size for c in qc.cregs]),
        "quantum_registers":quantum_registers,
        "classical_registers":classical_registers
    }
    for i in range(len(qc.data)):
        if qc.data[i].name == "barrier":
            pass
        elif qc.data[i].name != "measure":

            qreg = [r._register.name for r in qc.data[i].qubits]
            qubit = [q._index for q in qc.data[i].qubits]

            json_data["instructions"].append({"name":qc.data[i].name, 
                                              "qubits":[quantum_registers[k][q] for k,q in zip(qreg,qubit)],
                                              "params":qc.data[i].params
                                             })
        else:
            qreg = [r._register.name for r in qc.data[i].qubits]
            qubit = [q._index for q in qc.data[i].qubits]
            
            creg = [r._register.name for r in qc.data[i].clbits]
            bit = [b._index for b in qc.data[i].clbits]

            json_data["instructions"].append({"name":qc.data[i].name,
                                              "qubits":[quantum_registers[k][q] for k,q in zip(qreg,qubit)],
                                              "memory":[classical_registers[k][b] for k,b in zip(creg,bit)]
                                             })
                

    return json_data

from qiskit import QuantumCircuit
from qiskit.circuit import QuantumRegister, ClassicalRegister, CircuitInstruction, Instruction, Qubit, Clbit


def from_json_to_qc(circuit_dict):
    if isinstance(circuit_dict, QuantumCircuit):
        raise TypeError("Circuit provided is already a <class 'qiskit.circuit.quantumcircuit.QuantumCircuit'>.")
    elif isinstance(circuit_dict, dict):
        circuit = circuit_dict
    else:
        raise TypeError("Please provide circuit_dict as a dict object.")

    if all([k in circuit.keys() for k in ['instructions', 'num_qubits', 'num_clbits', 'classical_registers']]):
        instructions = circuit['instructions']
        num_qubits = circuit['num_qubits']
        num_clbits = circuit['num_clbits']
        classical_registers = circuit['classical_registers']
    else:
        raise KeyError("Circuit dict must have 'instructions', 'num_qbits' and 'num_clbits'.")
    
    qc = QuantumCircuit(num_qubits)

    for cr, lista in classical_registers.items():
        qc.add_register(ClassicalRegister(len(lista), cr))


    for instruction in instructions:
        if instruction['name'] != 'measure':
            inst = CircuitInstruction( 
                operation = Instruction(name = instruction['name'],
                                        num_qubits = len(instruction['qubits']),
                                        num_clbits = 0,
                                        params = instruction['params']
                                        ),
                qubits = (Qubit(QuantumRegister(num_qubits, 'q'), q) for q in instruction['qubits']),
                clbits = ()
                )
            qc.append(inst)
        elif instruction['name'] == 'measure':
            bit = instruction['memory'][0]
            for k,v in classical_registers.items():
                if bit in v:
                    reg = k
                    l = len(v)
                    clbit = v.index(bit)
            inst = CircuitInstruction( 
                operation = Instruction(name = instruction['name'],
                                        num_qubits = 1,
                                        num_clbits = 1,
                                        params = []
                                        ),
                qubits = (Qubit(QuantumRegister(num_qubits, 'q'), q) for q in instruction['qubits']),
                clbits = (Clbit(ClassicalRegister(l, reg), clbit),)
                )
            qc.append(inst)
    return qc






