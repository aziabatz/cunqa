#!/bin/bash

module --force purge
module load qmio/hpc gcc/12.3.0 qmio-tools/0.2.0-python-3.9.9 qiskit/1.2.4-python-3.9.9
python fakeqmio.py $1

