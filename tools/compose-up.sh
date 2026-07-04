#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

bash "$ROOT/tools/compose-config.sh"
bash "$ROOT/tools/compose-build.sh"

docker compose -f "$ROOT/infra/compose/docker-compose.yml" --profile infra --profile app up -d "$@"

echo "Stack is starting. Gateway: \${FB_HOST:-127.0.0.1}:\${GATEWAY_PORT:-3001}"
