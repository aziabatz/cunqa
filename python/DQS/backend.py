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


class QPU:
	
	def __init__(self, list_backends=0):
		pass

	def set_backend(self, sim="aer", method="statevector", noise_model=None, **kwargs):
		if sim == "aer":
			return AerSimulator(method=method, noise_model=noise_model, **kwargs)
		else:
			print("No valid simualtor.")

	def from_backend(self,backend):
		return backend
	
	def dist_backend(self, list_backends, dict_backend_QPUs=None):
		"""The dictionary that relates backends to QPUs is optional. 
		It would only make sense if we knew how to raise QPUs 
		with different resources."""

		print("Distribuimos los backends a los recursos")

	def run(self, circs, shots=1024, dict_circs_backend=0, **kwargs):
		job_res = self.run(circs, shots=shots)
		#res = res_aux.result()
		return job_res

#	def sum(self):
#		res = prueba.sum(2,3)
#		return res



