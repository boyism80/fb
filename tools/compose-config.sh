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
TABLE_PUBLISH_DOWNLOAD_BASE_URL="${TABLE_PUBLISH_DOWNLOAD_BASE_URL:-}"
SCRIPT_PUBLISH_DOWNLOAD_BASE_URL="${SCRIPT_PUBLISH_DOWNLOAD_BASE_URL:-}"

render_template() {
    local template="$1"
    local output="$2"
    # URLs contain '/', so these substitutions use '|' as the delimiter.
    sed -e "s/__FB_HOST__/${FB_HOST}/g" \
        -e "s|__TABLE_PUBLISH_DOWNLOAD_BASE_URL__|${TABLE_PUBLISH_DOWNLOAD_BASE_URL}|g" \
        -e "s|__SCRIPT_PUBLISH_DOWNLOAD_BASE_URL__|${SCRIPT_PUBLISH_DOWNLOAD_BASE_URL}|g" \
        "$template" > "$output"
    echo "Generated $output (FB_HOST=$FB_HOST)"
}

render_template "$COMPOSE_DIR/config/gateway/config.json.template" "$COMPOSE_DIR/config/gateway/config.json"
render_template "$COMPOSE_DIR/config/login/config.json.template" "$COMPOSE_DIR/config/login/config.json"
render_template "$COMPOSE_DIR/config/game/config.json.template" "$COMPOSE_DIR/config/game/config.json"
render_template "$COMPOSE_DIR/config/game-cross/config.json.template" "$COMPOSE_DIR/config/game-cross/config.json"
