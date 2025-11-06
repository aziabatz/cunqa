#!/usr/bin/env bash
set -euo pipefail

NETWORK_NAME=${1:-cunqa-net}

if docker network inspect "$NETWORK_NAME" >/dev/null 2>&1; then
  echo "overlay network '$NETWORK_NAME' already exists"
  exit 0
fi

docker network create -d overlay --attachable "$NETWORK_NAME"
echo "created overlay network '$NETWORK_NAME'"


