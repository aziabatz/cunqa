import json

_qpus = []

def get_qpus():
        global _qpus
        list_qpus = []
        with open("qpus.json") as server_data:
                data = json.load(server_data)
                for key in list(data.keys()):
                        list_qpus.append(QPU(id=key,hostname=data[key][0]['hostname'], port=data[key][0]['port']))

        _qpus = list_qpus
        return list_qpus


def transpile(circ, qpu, **kwargs):
        trans_circ = transpile(circ, backend=qpu.backend, **kwargs)
        return trans_circ


def set_mult_backends(list_backends):
        if len(_qpus)==0:
                print("QPUs don't exist")
        elif len(list_backends) != len(_qpus):
                print("Different number of backends and qpus")
        elif len(list_backends)==len(_qpus):
                for i in range(len(list_backends)): _qpus[i].backend = list_backends[i]
#       eleif
#               for i in range(len(list_backends)): list_qpus[i].backend = list_backends[i] 

def run(circ, qpu=None, id=None, **kwargs):
        if qpu != None:
                result = qpu.backend.run(circ, **kwargs)
                #result = list(filter(lambda qpu: qpu.id == id, list_qpus))[0].run(circ,**k$
                return result

        elif id == None:
                print("Nor QPU nor id specified")

        elif len(_qpus)==0:
                print("There are not QPUs defined ")

        else:
                qpu_backend = list(filter(lambda qpu: qpu.id == id, _qpus))[0].backend
                result = qpu_backend.run(circ, **kwargs)

        return result
