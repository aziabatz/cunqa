#!/bin/bash
cd ../

if [ ! -f ".gitmodules" ] || ! grep -q 'aer' ".gitmodules"; then
    # How to load a new module
    git submodule add --name aer-cpp git@github.com:Qiskit/qiskit-aer.git src/third-party/aer-cpp
    # How to update a module if --recursive option was forgotten in clone
    git submodule update src/third-party/spdlog
    git submodule update --init --recursive src/third-party/mqt-ddsim
fi

cd src/third-party/aer-cpp/ && git config core.sparsecheckout true && cd ../../../
echo src/ >> .git/modules/aer-cpp/info/sparse-checkout 
cd src/third-party/aer-cpp/ && git read-tree -mu HEAD 

cd ../../../

cd src/third-party/argparse && git config core.sparsecheckout true && cd ../../../
echo include/argparse/argparse.hpp >> .git/modules/argparse/info/sparse-checkout 
cd src/third-party/argparse && git read-tree -mu HEAD 

cd ../../../

cd src/third-party/zmq && git config core.sparsecheckout true && cd ../../../
echo zmq.hpp >> .git/modules/zmq/info/sparse-checkout 
echo zmq_addon.hpp >> .git/modules/zmq/info/sparse-checkout 
cd src/third-party/zmq && git read-tree -mu HEAD 

#git submodule add --name libzmq git@github.com:zeromq/libzmq.git src/third-party/libzmq
#git submodule init
#git submodule update

#cd src/third-party/libzmq
#git rev-parse v4.3.4
#git reset --hard v4.3.4