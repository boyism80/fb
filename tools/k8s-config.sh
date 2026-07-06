#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OVERLAY="$ROOT/infra/k8s/overlays/dev"
CONFIG="$OVERLAY/config"
FB_HOST="${FB_HOST:-127.0.0.1}"

if [[ -f "$ROOT/infra/k8s/config/fb-host.env" ]]; then
    # shellcheck disable=SC1091
    source "$ROOT/infra/k8s/config/fb-host.env"
    FB_HOST="${FB_HOST:-127.0.0.1}"
fi

mkdir -p "$CONFIG"/{gateway,login-dev,game-dev-0,internal,log-dev,write-back-dev,marketplace,matchmaking,admin-tool}

render_cpp_config() {
    local service="$1"
    local template="$ROOT/infra/k8s/config/${service}/config.json.template"
    local output="$CONFIG/${service}/config.json"
    sed "s/__FB_HOST__/${FB_HOST}/g" "$template" > "$output"
    echo "Updated $output (FB_HOST=$FB_HOST)"
}

render_cpp_config gateway
render_cpp_config login-dev
render_cpp_config game-dev-0

cp "$ROOT/infra/k8s/config/internal/appsettings.k8s.json" "$CONFIG/internal/"
cp "$ROOT/infra/k8s/config/log-dev/appsettings.json" "$CONFIG/log-dev/"
cp "$ROOT/infra/k8s/config/write-back-dev/appsettings.json" "$CONFIG/write-back-dev/"
cp "$ROOT/infra/k8s/config/marketplace/appsettings.k8s.json" "$CONFIG/marketplace/"
cp "$ROOT/infra/k8s/config/matchmaking/appsettings.k8s.json" "$CONFIG/matchmaking/"
cp "$ROOT/infra/k8s/config/admin-tool/appsettings.k8s.json" "$CONFIG/admin-tool/"

echo "Apply with: kubectl apply -k $OVERLAY"
