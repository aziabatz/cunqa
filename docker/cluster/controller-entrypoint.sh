#!/usr/bin/env bash

## ESTE ES EL ENTRY DEL NODO HEAD/MASTER


set -euo pipefail



SLURM_CLUSTER_NAME=${SLURM_CLUSTER_NAME:-cunqa}
SLURM_CONTROL_MACHINE=${SLURM_CONTROL_MACHINE:-controller}
NODE_PREFIX=${SLURM_NODE_PREFIX:-worker-}
N_WORKERS=${N_WORKERS:-2}
CPUS_PER_NODE=${CPUS_PER_NODE:-2}
SLURM_PARTITION_NAME=${SLURM_PARTITION_NAME:-debug}

STATE_SAVE_DIR=${STATE_SAVE_DIR:-/var/spool/slurmctld}
SLURMD_SPOOL_DIR=${SLURMD_SPOOL_DIR:-/var/spool/slurmd}

mkdir -p /etc/slurm "$STATE_SAVE_DIR" "$SLURMD_SPOOL_DIR"

install -d -m 0700 -o munge -g munge /etc/munge /var/lib/munge /var/log/munge
install -d -m 0755 -o munge -g munge /run/munge
chown -R munge:munge /etc/munge /var/lib/munge /var/log/munge /run/munge
chmod 0700 /etc/munge /var/lib/munge /var/log/munge
chmod 0755 /run/munge

if [ -f /run/secrets/munge_key ]; then
  install -m 0600 -o munge -g munge /run/secrets/munge_key /etc/munge/munge.key
elif [ -f "${STORE:-/workspace/runtime}/.munge/munge.key" ]; then
  install -m 0600 -o munge -g munge "${STORE:-/workspace/runtime}/.munge/munge.key" /etc/munge/munge.key
elif [ ! -f /etc/munge/munge.key ]; then
  dd if=/dev/urandom bs=1 count=1024 of=/etc/munge/munge.key
  chmod 0600 /etc/munge/munge.key
  chown munge:munge /etc/munge/munge.key
fi

echo "[controller] generating slurm.conf (workers: ${N_WORKERS}, cpus/node: ${CPUS_PER_NODE})"
sed -e "s|__CLUSTER_NAME__|$SLURM_CLUSTER_NAME|g" \
    -e "s|__CONTROL_MACHINE__|$SLURM_CONTROL_MACHINE|g" \
    -e "s|__NODE_PREFIX__|$NODE_PREFIX|g" \
    -e "s|__NODE_RANGE__|1-$N_WORKERS|g" \
    -e "s|__CPUS_PER_NODE__|$CPUS_PER_NODE|g" \
    -e "s|__STATE_SAVE_DIR__|$STATE_SAVE_DIR|g" \
    -e "s|__SLURMD_SPOOL_DIR__|$SLURMD_SPOOL_DIR|g" \
    -e "s|__PARTITION_NAME__|$SLURM_PARTITION_NAME|g" \
    /opt/cunqa/docker/cluster/slurm/slurm.conf.tmpl > /etc/slurm/slurm.conf

if [ -n "${COMPUTE_NODES:-}" ]; then
  nodes_line="NodeName=${COMPUTE_NODES} CPUs=${CPUS_PER_NODE} State=UNKNOWN"
  part_line="PartitionName=${SLURM_PARTITION_NAME} Nodes=${COMPUTE_NODES} Default=YES MaxTime=INFINITE State=UP"
  sed -i -E "s|^NodeName=.*|${nodes_line}|" /etc/slurm/slurm.conf
  sed -i -E "s|^PartitionName=.*|${part_line}|" /etc/slurm/slurm.conf
fi

install -m 0644 /opt/cunqa/docker/cluster/slurm/cgroup.conf /etc/slurm/cgroup.conf

echo "[controller] starting munged as user 'munge'"
su -s /bin/sh -c "/usr/sbin/munged" munge
for i in $(seq 1 30); do
  if [ -S /run/munge/munge.socket.2 ]; then
    echo "[controller] munge socket is ready"
    break
  fi
  echo "[controller] waiting for munge socket... ($i)"
  sleep 0.5
done

echo "[controller] starting slurmctld"
exec slurmctld -Dvv
