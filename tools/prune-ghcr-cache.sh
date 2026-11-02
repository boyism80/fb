#!/bin/bash

set -euo pipefail

OWNER="${GHCR_OWNER:-boyism80}"
CACHE_MAX_DAYS="${CACHE_MAX_DAYS:-7}"

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

if ! packages=$(gh api --paginate "/users/${OWNER}/packages?package_type=container" \
    --jq '.[] | select(.name | startswith("fb/")) | .name | ltrimstr("fb/")'); then
    echo "Failed to list GHCR packages of ${OWNER}"
    exit 1
fi

failed=()
for pkg in ${packages}; do
    encoded="fb%2F${pkg}"
    echo "Checking ghcr.io/${OWNER}/fb/${pkg}..."

    if ! version_ids=$(gh api --paginate "/users/${OWNER}/packages/container/${encoded}/versions" \
        --jq ".[] | select((.metadata.container.tags | length) == 0) | select((.created_at | fromdateiso8601) < ${CUTOFF_EPOCH}) | .id"); then
        echo "  Failed to list versions, skipping"
        failed+=("${pkg}")
        continue
    fi

    for version_id in ${version_ids}; do
        echo "  Deleting untagged version ${version_id}"
        gh api --method DELETE "/users/${OWNER}/packages/container/${encoded}/versions/${version_id}" || true
    done
done

if [ ${#failed[@]} -gt 0 ]; then
    echo "Failed to prune: ${failed[*]}"
    exit 1
fi
