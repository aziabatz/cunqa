#!/usr/bin/env bash
set -euo pipefail

SECRET_NAME=${1:-munge_key}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

keyfile="$tmpdir/munge.key"
dd if=/dev/urandom bs=1 count=1024 of="$keyfile" >/dev/null 2>&1

if docker secret inspect "$SECRET_NAME" >/dev/null 2>&1; then
  echo "secret '$SECRET_NAME' already exists"
  exit 0
fi

docker secret create "$SECRET_NAME" "$keyfile"
echo "created docker swarm secret '$SECRET_NAME'"

