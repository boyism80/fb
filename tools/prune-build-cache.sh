#!/bin/bash

set -euo pipefail

CACHE_MAX_AGE="${CACHE_MAX_AGE:-168h}"

docker_cmd() {
    if sudo docker info >/dev/null 2>&1; then
        sudo docker "$@"
    elif docker info >/dev/null 2>&1; then
        docker "$@"
    else
        echo "Docker is not available" >&2
        exit 1
    fi
}

echo "Pruning local BuildKit cache older than ${CACHE_MAX_AGE}..."
docker_cmd buildx prune --filter "until=${CACHE_MAX_AGE}" --force || true
docker_cmd builder prune --filter "until=${CACHE_MAX_AGE}" --force || true
