#import json

def from_qc_to_json(qc):
    qc_dict = {
        
        "qubits":qc.num_qubits,
        "bits":qc.num_clbits,
        "circuit":[]
    }
    for i in range(len(qc.data)):
        if qc.data[i].name == "barrier":
            pass
        elif qc.data[i].name != "measure":
            qc_dict["circuit"].append({"name":qc.data[i].name, 
                                              "qubits":[qc.data[i].qubits[j]._index for j in range(len(qc.data[i].qubits))],
                                              "params":qc.data[i].params
                                             })
        else:
            qc_dict["circuit"].append({"name":qc.data[i].name, 
                                              "qubits":[qc.data[i].qubits[j]._index for j in range(len(qc.data[i].qubits))],
                                              "memory":[qc.data[i].clbits[j]._index for j in range(len(qc.data[i].clbits))]
                                             })

    #qc_json = json.dumps(qc_dict)

    return qc_dict #qc_json

