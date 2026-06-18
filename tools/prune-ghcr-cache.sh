#!/bin/bash

set -euo pipefail

OWNER="${GHCR_OWNER:-boyism80}"
CACHE_MAX_DAYS="${CACHE_MAX_DAYS:-7}"
PACKAGES=(
    data
    build
    gateway
    login
    game
    bot
    internal
    write-back
    admin-tool
    log
    marketplace
)

if ! command -v gh >/dev/null 2>&1; then
    echo "gh CLI not found, skipping GHCR cache prune"
    exit 0
fi

if ! gh auth status >/dev/null 2>&1; then
    echo "gh is not authenticated, skipping GHCR cache prune"
    exit 0
fi

if date -u -d "${CACHE_MAX_DAYS} days ago" +%s >/dev/null 2>&1; then
    CUTOFF_EPOCH=$(date -u -d "${CACHE_MAX_DAYS} days ago" +%s)
else
    CUTOFF_EPOCH=$(date -u -v-"${CACHE_MAX_DAYS}"d +%s)
fi

echo "Pruning untagged GHCR package versions older than ${CACHE_MAX_DAYS} days..."

for pkg in "${PACKAGES[@]}"; do
    encoded="fb%2F${pkg}"
    echo "Checking ghcr.io/${OWNER}/fb/${pkg}..."

    gh api --paginate "/users/${OWNER}/packages/container/${encoded}/versions" \
        --jq ".[] | select((.metadata.container.tags | length) == 0) | select((.created_at | fromdateiso8601) < ${CUTOFF_EPOCH}) | .id" \
        | while read -r version_id; do
            if [ -z "${version_id}" ]; then
                continue
            fi

            echo "  Deleting untagged version ${version_id}"
            gh api --method DELETE "/users/${OWNER}/packages/container/${encoded}/versions/${version_id}" || true
        done
done
