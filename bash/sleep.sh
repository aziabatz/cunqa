#!/bin/bash

if (($SLURM_LOCALID == "0")); then
    echo "Hola desde la task $SLURM_LOCALID"
fi

sleep 2