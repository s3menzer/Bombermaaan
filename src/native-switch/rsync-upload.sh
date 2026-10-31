#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"${SCRIPT_DIR}/docker-build.sh"

# Upload both NROs before the manifest. This ordering prevents a client from
# seeing metadata for an artifact that is not yet present on the server.
TARGET="${RSYNC_TARGET:-pi:/var/www/html/switch/Bombermaaan/}"

if ! command -v rsync >/dev/null 2>&1; then
    echo "[ERROR] rsync was not found." >&2
    exit 2
fi

if [[ ! -f "${SCRIPT_DIR}/game.nro" ||
      ! -f "${SCRIPT_DIR}/Bombermaaan.nro" ||
      ! -f "${SCRIPT_DIR}/Bombermaaan.json" ]]; then
    echo "[ERROR] Missing game.nro, Bombermaaan.nro, or Bombermaaan.json." >&2
    echo "Build the Switch target first with ./build.sh." >&2
    exit 3
fi

RSYNC_ARGS=(
    --archive
    --verbose
    --human-readable
    --partial
    --progress
)

echo "Uploading game.nro to ${TARGET}"
rsync "${RSYNC_ARGS[@]}" \
    "${SCRIPT_DIR}/game.nro" \
    "${TARGET}"

echo "Uploading Bombermaaan.nro to ${TARGET}"
rsync "${RSYNC_ARGS[@]}" \
    "${SCRIPT_DIR}/Bombermaaan.nro" \
    "${TARGET}"

echo "Uploading Bombermaaan.json to ${TARGET}"
rsync "${RSYNC_ARGS[@]}" \
    "${SCRIPT_DIR}/Bombermaaan.json" \
    "${TARGET}"

echo "Upload complete."
