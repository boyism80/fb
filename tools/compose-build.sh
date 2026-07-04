#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
COMPOSE_FILE="$ROOT/infra/compose/docker-compose.yml"
ENV_FILE="$ROOT/infra/compose/.env"

if [[ -f "$ENV_FILE" ]]; then
    set -a
    # shellcheck disable=SC1090
    source "$ENV_FILE"
    set +a
fi

BUILD_TYPE="${BUILD_TYPE:-Release}"

echo "Building fb/data:local..."
docker buildx build --load -t fb/data:local -f "$ROOT/server/fb/data/Dockerfile" "$ROOT"

echo "Building fb/build:local (this may take a long time on first run)..."
docker buildx build --load \
    --build-arg BUILD_TYPE="$BUILD_TYPE" \
    -t fb/build:local \
    -f "$ROOT/server/fb/Dockerfile" \
    "$ROOT"

echo "Building application images via compose..."
docker compose -f "$COMPOSE_FILE" --profile app build

echo "Done. Images tagged fb/*:local are ready."
