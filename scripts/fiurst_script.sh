#!/bin/bash

git submodule add git@github.com:Qiskit/qiskit-aer.git ../src/third-party/aer-cpp 
cd /src/third-party/aer-cpp
git config core.sparsecheckout true 
echo src/ >> .git/modules/aer-cpp/info/sparse-checkout 
git read-tree -mu HEAD 
git add -A
git commit -m 'add qiskit aer cpp as submodule/sparse-checkout' 