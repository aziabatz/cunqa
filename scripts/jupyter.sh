#!/bin/bash

export INSTALL_PATH=$STORE/works/cunqa/installation
 
export PATH=$PATH:$INSTALL_PATH/bin
 
export INFO_PATH=$STORE/.api_simulator/qpus.json

export LD_PRELOAD=$INSTALL_PATH/lib64/libzmq.so
IP=$(ip route get 1.2.3.4 | head -1 | awk '{print $7}')
LD_DEBUG=all jupyter-notebook --ip $IP &> jup.out &
export LD_PRELOAD=""