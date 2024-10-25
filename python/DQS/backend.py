import prueba
from qiskit import QuantumCircuit
#from qiskit.providers import Backend
from qiskit_aer import AerSimulator


class Backend:
	def __init__(self):
		pass
		#super().__init__()

	def set_backend(self, method="statevector", noise_model=None):
		return AerSimulator(method=method, noise_model = noise_model)


class DQS:
	
	def __init__(self, list_backends=0):
		pass

	def dist_backend(self, list_backends, dict_backend_QPUs=None):
		"""The dictionary that relates backends to QPUs is optional. 
		It would only make sense if we knew how to raise QPUs 
		with different resources."""

		print("Distribuimos los backends a los recursos")

	def run(self, circuitos, backends, dict_circuito_backend, **kwargs ):
		pass

	def sum(self):
		res = prueba.sum(2,3)
		return res


class heredar(AerSimulator):
	def __init__(self):
		super().__init__()
		#aux = AerSimulator()
		#self.self = AerSimulator()
		self.__class__ = AerSimulator

