import zmq
import struct

context = zmq.Context()

#  Socket to talk to server
print("Connecting to hello world server…")
socket = context.socket(zmq.REQ)
socket.connect("tcp://10.120.7.4:5550")

#  Do 10 requests, waiting each time for a response
#for request in range(10):
#    print(f"Sending request {request} …")
socket.send(b"ping")

    #  Get the reply.
#    message = socket.recv()
#    task_id = struct.unpack("!i",message)[0]
#    print(f"Id de la tarea: {task_id}")
