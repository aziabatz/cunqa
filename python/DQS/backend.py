from qiskit import QuantumCircuit, transpile
#from qiskit.providers import Backend
from qiskit_aer import AerSimulator
import json
from cluster import SLURMJob

=======

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
#	eleif
#		for i in range(len(list_backends)): list_qpus[i].backend = list_backends[i] 


def run(circ, qpu=None, id=None, **kwargs):
	if qpu != None:
		result = qpu.backend.run(circ, **kwargs)
		#result = list(filter(lambda qpu: qpu.id == id, list_qpus))[0].run(circ,**kwargs)
		return result

	elif id == None:
		print("Nor QPU nor id specified")

	elif len(_qpus)==0:
		print("There are not QPUs defined ")

	else:
		qpu_backend = list(filter(lambda qpu: qpu.id == id, _qpus))[0].backend
		result = qpu_backend.run(circ, **kwargs)

	return result


class QPU:
	def __init__(self, backend=None,id=None, hostname=None, port=None):
		self.backend = AerSimulator()
		self.id = id
		self.hostname = hostname
		self.port = port


	def run(self, circ, **kwargs):
		result = self.backend.run(circ, **kwargs)
		return result

#========================================================================================================================================
#========================================================================================================================================
#========================================================================================================================================

class QPU_:
    def __init__(self, id_=None, server_endpoint=None,backend=None):

        """
        Initializes th QPU class.

        Attributes:
        -----------
        id_ (int): Id assigned to the qpu, simply a int from 0 to n_qpus-1.
        
        server_endpoint (str): Ip and port route that identifies the server that represents the qpu itself which the worker will
            communicate with.
            
        backend (<class backend.Backend>): Backend object <====== AUN POR DEFINIR PORQUE DEPENDE DE LO DE ABAJO !!!!!!!
            By default, it will be set to AerSimulator().
        
        """
        if id_ is not None and type(id_) is int:
            self.id=id_
        else:
            raise ValueError("id not correct")


        # asignamos server
        if server_endpoint is not None and type(server_endpoint) is str:
            self.server_endpoint=server_endpoint
        else:
            raise ValueError("Server not assigned")
    
        # asignamos backend, será el nombre del archivo de configuración para FakeQmio
        if backend is None:
            sef.backend=AerSimulator()
        else:
            self.backend=backend

class Cluster:
    def __init__(self,nqpus=1, config=None, slurm_config=None):
        self.nqpus=nqpus
        default_config={}
        default_slurm_config={"cores":10, "ntasks":self.nqpus, "walltime":"00:02:00","mem_per_cpu":"1G"}
        if config is None:
            self.config=default_config
        else:
            self.config=config
        if slurm_config is None:
            self.slurm_config=default_slurm_config
        else:
            self.slurm_config=slurm_config

        # creamos el script para levantar el cluster y lanzamos el trabajo
        job=SLURMJob(parameter_dict=self.slurm_config); job.generate_script(); job.launch()

        while job.status()=='RUNNING':
            return






#==============================================================================================================================================
#==============================================================================================================================================
#==============================================================================================================================================
	def set_backend(self, backend):
		self.backend = backend

#		if qpu:
#			qpu.backend = backend
#		elif (list_qpus != None)  & (id != None):
#			list(filter(lambda qpu: qpu.id == id, list_qpus))[0].backend = backend
#		else:
#			print("Cannot assign backend")





###############################################################################
################################ PRUEBAS ######################################
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



