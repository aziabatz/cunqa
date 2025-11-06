#!/usr/bin/env bash
set -euo pipefail

# Borra todo y reconstruye 

ROOT_DIR="$(pwd)"
ENV_FILE="${ENV_FILE:-${ROOT_DIR}/docker/cluster/.cluster.env}"

if [[ ! -f "${ENV_FILE}" ]]; then
  echo " Falta ${ENV_FILE}. Copia y edita docker/cluster/.cluster.env." >&2
  exit 1
fi

set -a
source "${ENV_FILE}"
set +a

STACK_NAME="${STACK_NAME:-cunqa}"
CUNQA_IMAGE="${CUNQA_IMAGE:-cunqa:dev}"

ADVERTISE_ADDR="${ADVERTISE_ADDR:-}"
if [ -z "$ADVERTISE_ADDR" ]; then
  ADVERTISE_ADDR=$(hostname -I 2>/dev/null | awk '{for(i=1;i<=NF;i++) if ($i!~ /^127\./) {print $i; exit}}')
fi
if [ -z "$ADVERTISE_ADDR" ]; then
  ADVERTISE_ADDR=127.0.0.1
fi

NFS_SERVER="${NFS_SERVER:-$ADVERTISE_ADDR}"
NFS_EXPORT_PATH="${NFS_EXPORT_PATH:-/}"
NFS_VERSION="${NFS_VERSION:-4.2}"
NFS_DOCKER_VOLUME="${NFS_DOCKER_VOLUME:-cunqa_export}"
NFS_CONTAINER_NAME="${NFS_CONTAINER_NAME:-cunqa-nfs}"

WORKER_REPLICAS="${WORKER_REPLICAS:-1}"
CPUS_PER_NODE="${CPUS_PER_NODE:-2}"
SLURM_CLUSTER_NAME="${SLURM_CLUSTER_NAME:-cunqa}"
SLURM_PARTITION_NAME="${SLURM_PARTITION_NAME:-debug}"
COMPUTE_NODES="${COMPUTE_NODES:-}"

JUPYTER_TOKEN="${JUPYTER_TOKEN:-cunqa}"
JUPYTER_PORT="${JUPYTER_PORT:-8888}"

echo " Limpieza total de recursos previos del stack '${STACK_NAME}'"

# 1) Eliminar stack 
docker stack rm "${STACK_NAME}" 2>/dev/null || true
for i in $(seq 1 10); do
  if docker stack services "${STACK_NAME}" >/dev/null 2>&1; then
    echo " Esperando a que el stack se elimine... (${i})"
    sleep 1
  else
    break
  fi
done

# 2) Eliminar nfs
docker rm -f "${NFS_CONTAINER_NAME}" 2>/dev/null || true
for v in "${STACK_NAME}_cunqa_shared" "${STACK_NAME}_controller_state" "${STACK_NAME}_worker_state"; do
  docker volume rm "${v}" 2>/dev/null || true
done

# 3) Eliminar red y secretos
docker network rm cunqa-net 2>/dev/null || true
docker secret rm munge_key 2>/dev/null || true

echo " Recreando recursos"

if ! docker info --format '{{.Swarm.LocalNodeState}}' 2>/dev/null | grep -qiE '^active$'; then
  echo " Inicializando Docker Swarm en ${ADVERTISE_ADDR}"
  docker swarm init --advertise-addr "${ADVERTISE_ADDR}" >/dev/null 2>&1 || true
fi
docker network create -d overlay --attachable cunqa-net 2>/dev/null || true

TMPDIR="$(mktemp -d)"; trap 'rm -rf "${TMPDIR}"' EXIT
dd if=/dev/urandom bs=1 count=1024 of="${TMPDIR}/munge.key" >/dev/null 2>&1 || true
docker secret create munge_key "${TMPDIR}/munge.key" 2>/dev/null || true

docker volume create "${NFS_DOCKER_VOLUME}" >/dev/null 2>&1 || true

docker pull itsthenetwork/nfs-server-alpine:latest >/dev/null 2>&1 || true
docker run -d --name "${NFS_CONTAINER_NAME}" --privileged --restart=unless-stopped \
  --network host \
  -e SHARED_DIRECTORY=/export \
  -v "${NFS_DOCKER_VOLUME}:/export:rw" \
  itsthenetwork/nfs-server-alpine:latest >/dev/null

sleep 1
echo " Logs del NFS:"
docker logs "${NFS_CONTAINER_NAME}" | tail -n +1 || true

# magia de munge, es lo que hay....
echo " Verificando clave MUNGE en el NFS export"
docker pull busybox:latest >/dev/null 2>&1 || true
docker run --rm -v "${NFS_DOCKER_VOLUME}:/export" busybox:latest sh -c '
  set -eu;
  mkdir -p /export/.munge;
  if [ ! -f /export/.munge/munge.key ]; then
    echo "[init] creando /export/.munge/munge.key";
    dd if=/dev/urandom bs=1 count=1024 of=/export/.munge/munge.key >/dev/null 2>&1;
    chmod 400 /export/.munge/munge.key;
  else
    echo "[init] clave ya existe en /export/.munge/munge.key";
  fi
'

EXPECTED_DEVICE=":${NFS_EXPORT_PATH}"
if docker volume inspect "${STACK_NAME}_cunqa_shared" >/dev/null 2>&1; then
  CURR_DEV="$(docker volume inspect -f '{{ index .Options "device" }}' "${STACK_NAME}_cunqa_shared" 2>/dev/null || echo "")"
  if [[ "${CURR_DEV}" != "${EXPECTED_DEVICE}" ]]; then
    echo " Volumen ${STACK_NAME}_cunqa_shared device=${CURR_DEV}, esperado=${EXPECTED_DEVICE}. Forzando recreación."
    docker volume rm "${STACK_NAME}_cunqa_shared" 2>/dev/null || true
  fi
fi

# Exportar variables y desplegar
export NFS_SERVER NFS_EXPORT_PATH NFS_VERSION CUNQA_IMAGE \
       WORKER_REPLICAS CPUS_PER_NODE SLURM_CLUSTER_NAME SLURM_PARTITION_NAME COMPUTE_NODES \
       JUPYTER_TOKEN JUPYTER_PORT

echo " Construyendo imagen ${CUNQA_IMAGE} (puede tardar)"
docker build -t "${CUNQA_IMAGE}" "${ROOT_DIR}"

echo " Desplegando stack ${STACK_NAME}"
docker stack deploy -c "${ROOT_DIR}/docker/cluster/docker-compose.cluster.yml" "${STACK_NAME}"

echo " Servicios:"
docker stack services "${STACK_NAME}" || true
echo " Tareas:"
docker stack ps "${STACK_NAME}" || true

echo " Hecho. Tips:"
echo "  - Espera ~20-30s y revisa logs:"
echo "      docker service logs --tail 120 ${STACK_NAME}_controller"
echo "      docker service logs --tail 120 ${STACK_NAME}_worker"
echo "  - Jupyter: http://127.0.0.1:${JUPYTER_PORT}/?token=${JUPYTER_TOKEN}"
