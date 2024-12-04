#!/bin/bash

jq --arg SLURM_LOCALID "$SLURM_LOCALID" 'del(.[$SLURM_LOCALID])' $CONFIG_PATH > tmp_qpu.json && mv tmp_qpu.json $CONFIG_PATH
