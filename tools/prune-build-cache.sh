#!/bin/bash

set -euo pipefail

CACHE_MAX_AGE="${CACHE_MAX_AGE:-168h}"
CACHE_MAX_USED="${CACHE_MAX_USED:-50GB}"
BUILDER_NAME="${BUILDER_NAME:-fb-builder}"

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

# Jobs before the shared builder name left one builder-<uuid> per run behind.
echo "Removing auto-generated buildx builders..."
for builder in $(docker_cmd buildx ls --format '{{.Name}}' | grep -E '^builder-[0-9a-f-]{36}$' || true); do
    docker_cmd buildx rm -f "${builder}" || true
done

echo "Pruning local BuildKit cache older than ${CACHE_MAX_AGE}, capped at ${CACHE_MAX_USED}..."
docker_cmd builder prune --all --filter "until=${CACHE_MAX_AGE}" --force || true
docker_cmd builder prune --all --max-used-space "${CACHE_MAX_USED}" --force || true
if docker_cmd buildx inspect "${BUILDER_NAME}" >/dev/null 2>&1; then
    docker_cmd buildx prune --builder "${BUILDER_NAME}" --all --filter "until=${CACHE_MAX_AGE}" --force || true
    docker_cmd buildx prune --builder "${BUILDER_NAME}" --all --max-used-space "${CACHE_MAX_USED}" --force || true
fi

echo "Pruning dangling images..."
docker_cmd image prune --force || true
