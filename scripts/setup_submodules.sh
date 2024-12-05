#!/bin/bash
cd ../

if [ ! -f ".gitmodules" ] || ! grep -q 'aer' ".gitmodules"; then
    git submodule add --name aer-cpp git@github.com:Qiskit/qiskit-aer.git src/third-party/aer-cpp
fi

cd src/third-party/aer-cpp/ && git config core.sparsecheckout true && cd ../../../
echo src/ >> .git/modules/aer-cpp/info/sparse-checkout 
cd src/third-party/aer-cpp/ && git read-tree -mu HEAD 
cd ../../../
cd src/third-party/argparse && git config core.sparsecheckout true && cd ../../../
echo include/argparse/argparse.hpp >> .git/modules/argparse/info/sparse-checkout 
cd src/third-party/argparse && git read-tree -mu HEAD 
