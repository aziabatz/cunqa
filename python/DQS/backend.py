from qiskit import QuantumCircuit
#from qiskit.providers import Backend
from qiskit_aer import AerSimulator
import json



def get_qpus():
	list_qpus = []
	with open("qpus.json") as server_data:
		data = json.load(server_data)
		for key in list(data.keys()):
			list_qpus.append(QPU(id=key,hostname=data[key][0]['hostname'], port=data[key][0]['port']))
	return list_qpus


def set_backend(backend, qpu=None, list_qpus=None, id=None):

	if qpu:
		qpu.backend = backend
	elif (list_qpus != None)  & (id != None):
		list(filter(lambda qpu: qpu.id == id, list_qpus))[0].backend = backend
	else:
		print("Cannot assign backend")


def set_mult_backends(list_backends, list_qpus=None):
	if len(list_backends) != len(list_qpus):
		print("Different number of backends and qpus")
	else:
		for i in range(len(list_backends)): list_qpus[i].backend = list_backends[i] 


def run(circ, qpu=None, list_qpus=None, id=None, **kwargs):
	if qpu != None:
		result = qpu.backend.run(circ, **kwargs)
		#result = list(filter(lambda qpu: qpu.id == id, list_qpus))[0].run(circ,**kwargs)
		return result

	elif (list_qpus != None) & (id != None):
		qpu_backend = list(filter(lambda qpu: qpu.id == id, list_qpus))[0].backend
		result = qpu_backend.run(circ, **kwargs)
		return result

	else:
		print("QPU not specified")





class QPU:
	def __init__(self, backend=None,id=None, hostname=None, port=None):
		self.backend = AerSimulator()
		self.id = id
		self.hostname = hostname
		self.port = port


	def run(self, circ, **kwargs):
		result = self.backend.run(circ, **kwargs)
		return result






###############################################################################


class DistQPU2:
	def __init__(self, list_qpus=None):
		self.list_qpus = list_qpus

	def set_qpus(self, list_qpus): # qpu = backend + server
		self.list_qpus = list_qpus

	def dist_run(self, list_circs, list_shots=None, **kwargs):
		#key, value = kwargs.items()
		result = [self.list_qpus[i].run(list_circs[i], shots=list_shots[i], **kwargs) for i in range(len(list_circs))]
		
		return result


class DistQPU:

	def __init__(self, num_QPUs=1):
		self.num_QPUs = num_QPUs
		self.backends = [AerSimulator()]*num_QPUs
		self.QPUs = None #Lista de QPUs (backend + server)

	def set_num_QPUs(self, num):
		""" Numero de QPUs que necesita el usuario"""
		self.num_QPUs = num
		self.backends = [AerSimulator()]*num


	def set_backends(self, list_backends):
		"""Lista de backends proporcionada por el usuario. Si el 
		Cluster est'a levantado, deben ser tantas como num_QPUs. En
		caso contrario numero de QPUs = numero de backends"""
		self.backends = list_backends
		#self.num_QPUs = len(list_backends)

	def assign_backends_to_resources(self, list_backends=None, list_QPUs=None):
		"""Se asigna cada backend a cada server levantado, cambiando
		el backend por defecto del server al correspondiente en la lista
		list_backends. Deber'ia devolver una lista de pares (backend,server_ID).
		Cada uno de estes pares es a lo que llamamos nosotros QPU"""
		if list_backends==None: list_backends=self.backends
		self.QPUs = None
		pass


	def dist_run(self, list_circs, list_shots=None, list_noise=None, **kwargs):
		"""Corre una lista de circuitos, cada uno en la QPU (backend +server)
		construida con los metodos anteriores. Hay que pensar los argumentos
		que puede pasar el usuario. De momento list_shots y list_noise. 
		Qu'e pasa si el user quiere pasar otros argumentos? Problema
		Qu'e pasa si otro simulador distinto de AER? Problema"""

		if list_shots==None: list_shots = [None]*len(list_circs) 
		if list_noise==None: list_noise = [None]*len(list_circs) 

		result = [self.backends[i].run(list_circs[i], 
					shots = list_shots[i], 
					noise_model = list_noise[i]) 
					for i in range(len(list_circs))]

		return result




#	def set_backend(self, sim="aer", method="statevector", noise_model=None, **kwargs):
#		if sim == "aer":
#			return AerSimulator(method=method, noise_model=noise_model, **kwargs)
#		else:
#			print("No valid simualtor.")


#	def set_backend(self, sim="aer", method="statevector", noise_model=None, **kwargs):
#		if sim == "aer":
#			backend = {
#				"sim": sim,
#				"method": method,
#				"noise_model": noise_model
#			}
#			return backend
#		else:
#			print("No valid simualtor.")



#	def from_backend(self,backend):
#		return backend


#	def dist_backend(self, list_backends=[], list_qpus=[], dict_backend_QPUs=None):
#		
#		if len(list_backends) != len(list_qpus):
#			print("Different number of backends and qpus")
#			pass
#		else:
#			job = []
#			for i in range(len(list_backends)):
#				job.append([list_backends[i],list_qpus[i]])
#			return job

#	def run(self, list_circs=[], list_backends=[], list_shots=[], dict_circs_backend=0, **kwargs):
#		
#		backends = [AerSimulator(method=backend["method"], noise_model=backend["noise_model"] ) for backend in list_backends]
#		job_res = [backends[i].run(list_circs[i], shots=list_shots[i]) for i in range(len(backends))]
#		#res = res_aux.result()
#		return job_res #backends_aux





#	def ejecuta(self, circ, shots = 1024, **kwargs):
#		res = self.run(circ, shots=shots)
#		return res

#	def sum(self):
#		res = prueba.sum(2,3)
#		return res



