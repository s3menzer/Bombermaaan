#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

: "${DEVKITPRO:?DEVKITPRO must be set (use docker-build.sh)}"
: "${DEVKITA64:?DEVKITA64 must be set (use docker-build.sh)}"

MIN_LIBNX_VERSION="4.10.0"

version_at_least() {
    local actual="$1"
    local required="$2"
    [[ "$(printf '%s\n%s\n' "$required" "$actual" | sort -V | tail -n 1)" == "$actual" ]]
}

libnx_version() {
    if [[ -n "${LIBNX_VERSION:-}" ]]; then
        printf '%s\n' "$LIBNX_VERSION"
        return 0
    fi

    local desc
    for desc in "$DEVKITPRO"/pacman/var/lib/pacman/local/libnx-*/desc; do
        [[ -f "$desc" ]] || continue
        sed -n '/^%VERSION%$/{n;p;q;}' "$desc"
        return 0
    done
    return 1
}

LIBNX_VERSION_DETECTED="$(libnx_version || true)"
if [[ -n "$LIBNX_VERSION_DETECTED" ]]; then
    if ! version_at_least "$LIBNX_VERSION_DETECTED" "$MIN_LIBNX_VERSION"; then
        cat >&2 <<EOF
libnx ${LIBNX_VERSION_DETECTED} is too old for the current Switch homebrew ABI.
Use libnx ${MIN_LIBNX_VERSION} or newer (the official devkitPro image is recommended).
EOF
        exit 4
    fi
    echo "Using libnx ${LIBNX_VERSION_DETECTED} (minimum ${MIN_LIBNX_VERSION})"
else
    echo "Warning: could not determine the libnx package version; verify it is >= ${MIN_LIBNX_VERSION}." >&2
fi

for tool in make elf2nro nacptool; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Required Switch build tool not found: ${tool}" >&2
        exit 4
    fi
done

VERSION_FILE="../Bombermaaan/version.hpp"
# Set AUTO_BUILD_NUMBER=0 for a reproducible rebuild without incrementing the
# numeric build stored in the version header.
AUTO_BUILD_NUMBER="${AUTO_BUILD_NUMBER:-1}"
VERSION_BACKUP=""

if [[ "${1:-}" != "clean" && "$AUTO_BUILD_NUMBER" != "0" ]]; then
    CURRENT_BUILD="$(sed -n \
        's/^#define BOMBERMAAAN_BUILD_NUMBER[[:space:]]*\([0-9][0-9]*\)$/\1/p' \
        "$VERSION_FILE")"

    if [[ ! "$CURRENT_BUILD" =~ ^[0-9]+$ ]]; then
        echo "Invalid BOMBERMAAAN_BUILD_NUMBER: ${CURRENT_BUILD}" >&2
        exit 3
    fi

    NEXT_BUILD=$((10#$CURRENT_BUILD + 1))
    VERSION_BACKUP="$(mktemp)"
    cp "$VERSION_FILE" "$VERSION_BACKUP"
    sed -i -E \
        "s/^#define BOMBERMAAAN_BUILD_NUMBER[[:space:]]+.*/#define BOMBERMAAAN_BUILD_NUMBER            ${NEXT_BUILD}/" \
        "$VERSION_FILE"
    sed -i -E \
        "s/(^#define BOMBERMAAAN_BUILD_STRING[[:space:]]+\")([0-9]+)(.*\")/\1${NEXT_BUILD}\3/" \
        "$VERSION_FILE"
    echo "Incremented Bombermaaan build number: ${CURRENT_BUILD} -> ${NEXT_BUILD}"
fi

if make -j"${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}" "$@"; then
    if [[ -n "$VERSION_BACKUP" ]]; then
        rm -f "$VERSION_BACKUP"
    fi
else
    BUILD_STATUS=$?
    if [[ -n "$VERSION_BACKUP" ]]; then
        cp "$VERSION_BACKUP" "$VERSION_FILE"
        rm -f "$VERSION_BACKUP"
        echo "Build failed; restored build ${CURRENT_BUILD}." >&2
    fi
    exit "$BUILD_STATUS"
fi

# Keep the manifest beside the NRO.  Upload the NRO before the manifest so a
# client never observes metadata for an artifact that is not available yet.
if [[ -f Bombermaaan.nro ]]; then
    VERSION="$(sed -n 's/^#define BOMBERMAAAN_VERSION_STRING[[:space:]]*"\(.*\)"/\1/p' ../Bombermaaan/version.hpp)"
    BUILD_NUMBER="$(sed -n 's/^#define BOMBERMAAAN_BUILD_NUMBER[[:space:]]*\([0-9][0-9]*\)$/\1/p' ../Bombermaaan/version.hpp)"
    VERSION="${VERSION:-0.0.0}"
    BUILD_NUMBER="${BUILD_NUMBER:-0}"
    MANIFEST_URL="${BOMBERMAAAN_MANIFEST_URL:-http://192.168.22.7/switch/bombermaaan.json}"
    DOWNLOAD_URL="${BOMBERMAAAN_DOWNLOAD_URL:-http://192.168.22.7/switch/Bombermaaan.nro}"

    if command -v sha256sum >/dev/null 2>&1; then
        SHA256="$(sha256sum Bombermaaan.nro | awk '{print $1}')"
    elif command -v shasum >/dev/null 2>&1; then
        SHA256="$(shasum -a 256 Bombermaaan.nro | awk '{print $1}')"
    else
        echo "sha256sum or shasum was not found." >&2
        exit 3
    fi

    cat > Bombermaaan.json <<EOF
{
  "name": "Bombermaaan",
  "platform": "switch",
  "version": "${VERSION}",
  "build": ${BUILD_NUMBER},
  "download": "${DOWNLOAD_URL}",
  "sha256": "${SHA256}"
}
EOF

    echo "Generated update manifest:"
    echo "  ${PWD}/Bombermaaan.json"
    echo "  manifest URL: ${MANIFEST_URL}"
elif [[ "${1:-}" == "clean" ]]; then
    rm -f Bombermaaan.json
fi
