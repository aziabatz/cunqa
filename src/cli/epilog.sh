#!/bin/bash

SLURM_TASK_ID="${SLURM_JOB_ID}_${SLURM_LOCALID}"
flock $INFO_PATH -c "/mnt/netapp1/Optcesga_FT2_RHEL7/2020/gentoo/22072020/usr/bin/jq 'del(.[\"$SLURM_TASK_ID\"])' $INFO_PATH > tmp_qpu_$SLURM_TASK_ID.json && mv tmp_qpu_$SLURM_TASK_ID.json $INFO_PATH"