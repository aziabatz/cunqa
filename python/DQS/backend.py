from qiskit import QuantumCircuit
#from qiskit.providers import Backend
from qiskit_aer import AerSimulator
#import prueba

#class Backend:
#	def __init__(self):
#		pass
#		#super().__init__()
#
#	def set_backend(self, method="statevector", noise_model=None):
#		return AerSimulator(method=method, noise_model = noise_model)


class DistQPU:
	
	def __init__(self, list_backends=0):
		pass

#	def set_backend(self, sim="aer", method="statevector", noise_model=None, **kwargs):
#		if sim == "aer":
#			return AerSimulator(method=method, noise_model=noise_model, **kwargs)
#		else:
#			print("No valid simualtor.")


	def set_backend(self, sim="aer", method="statevector", noise_model=None, **kwargs):
		if sim == "aer":
			backend = {
				"sim": sim,
				"method": method,
				"noise_model": noise_model
			}
			return backend
		else:
			print("No valid simualtor.")


	def from_backend(self,backend):
		return backend


	def dist_backend(self, list_backends=[], list_qpus=[], dict_backend_QPUs=None):
		"""The dictionary that relates backends to QPUs is optional. 
		It would only make sense if we knew how to raise QPUs 
		with different resources."""
		
		if len(list_backends) != len(list_qpus):
			print("Different number of backends and qpus")
			pass
		else:
			job = []
			for i in range(len(list_backends)):
				job.append([list_backends[i],list_qpus[i]])
			return job

	def run(self, list_circs=[], list_backends=[], list_shots=[], dict_circs_backend=0, **kwargs):
		
		backends = [AerSimulator(method=backend["method"], noise_model=backend["noise_model"] ) for backend in list_backends]
		job_res = [backends[i].run(list_circs[i], shots=list_shots[i]) for i in range(len(backends))]
		#res = res_aux.result()
		return job_res #backends_aux

	def ejecuta(self, circ, shots = 1024, **kwargs):
		res = self.run(circ, shots=shots)
		return res

#	def sum(self):
#		res = prueba.sum(2,3)
#		return res



