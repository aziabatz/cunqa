#
#   Hello World server in Python
#   Binds REP socket to tcp://*:5555
#   Expects b"Hello" from client, replies with b"World"
#

import time
import zmq
import os, sys, struct
from mpi4py import MPI

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()
#t_id = int(sys.argv[1])
#t_id = int(os.environ['SLURM_PROCID'])



context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5550")

cor = len(os.sched_getaffinity(0))
cor_str = struct.pack("!i",cor)

#print(t_id)

#task_id = int(sys.argv[1])
#task_id_str = struct.pack("!i", task_id)
while True:

	#print("Numero de CPUs segun os:", len(os.sched_getaffinity(0)))
	#cor = len(os.sched_getaffinity(0))
	message = socket.recv()
	print(f"Received request: {message}")

	#  Do some 'work'
	time.sleep(1)
	if rank == 0:
		f = open("prueba.txt", "w")
		f.write(f"Hola desde el proceso {rank}\n")
		f.close()
	else:
		f.open("prueba.txt", "w")
		f.write(f"Hola desde el proceso {rank}\n")
		f.close()
	#  Send reply back to client
	#socket.send(b"World")
	socket.send(cor_str)

