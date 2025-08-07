import os, sys
import numpy as np

# path to access c++ files
sys.path.append(os.getenv("HOME"))

from cunqa.qutils import getQPUs, qraise, qdrop
from cunqa.circuit import CunqaCircuit
from cunqa.mappers import run_distributed
from cunqa.qjob import gather

qpus  = getQPUs(local=False)

print([qpu._id for qpu in qpus])