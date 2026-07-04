#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
COMPOSE_DIR="$ROOT/infra/compose"
ENV_FILE="$COMPOSE_DIR/.env"

if [[ -f "$ENV_FILE" ]]; then
    set -a
    # shellcheck disable=SC1090
    source "$ENV_FILE"
    set +a
fi

FB_HOST="${FB_HOST:-127.0.0.1}"

render_template() {
    local template="$1"
    local output="$2"
    sed "s/__FB_HOST__/${FB_HOST}/g" "$template" > "$output"
    echo "Generated $output (FB_HOST=$FB_HOST)"
}

render_template "$COMPOSE_DIR/config/gateway/config.json.template" "$COMPOSE_DIR/config/gateway/config.json"
render_template "$COMPOSE_DIR/config/login/config.json.template" "$COMPOSE_DIR/config/login/config.json"
render_template "$COMPOSE_DIR/config/game/config.json.template" "$COMPOSE_DIR/config/game/config.json"
