#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SWITCH_DIR="$PROJECT_ROOT/src/native-switch"

"$SCRIPT_DIR/docker-build-switch.sh"

# Upload both NROs before the manifest so clients never see metadata for an
# artifact that is not yet available on the server.
TARGET="${RSYNC_TARGET:-pi:/var/www/html/switch/Bombermaaan/}"

if ! command -v rsync >/dev/null 2>&1; then
    echo "[ERROR] rsync was not found." >&2
    exit 2
fi

for artifact in game.nro Bombermaaan.nro Bombermaaan.json; do
    if [[ ! -f "$SWITCH_DIR/$artifact" ]]; then
        echo "[ERROR] Missing $artifact in $SWITCH_DIR." >&2
        echo "Build the Switch target first with ./scripts/docker-build-switch.sh." >&2
        exit 3
    fi
done

RSYNC_ARGS=(
    --archive
    --verbose
    --human-readable
    --partial
    --progress
)

for artifact in game.nro Bombermaaan.nro Bombermaaan.json; do
    echo "Uploading $artifact to $TARGET"
    rsync "${RSYNC_ARGS[@]}" "$SWITCH_DIR/$artifact" "$TARGET"
done

echo "Upload complete."
