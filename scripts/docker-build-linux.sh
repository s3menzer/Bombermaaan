#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE="${LINUX_DOCKER_IMAGE:-bombermaaan-linux-builder:latest}"
BUILD_DIR="${BUILD_DIR:-build-linux}"

build_linux() {
  local jobs="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"
  cd "$PROJECT_ROOT"
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  # Generated dependency files may contain absolute include paths from a
  # previous Windows/container build. Remove them before changing toolchains.
  make -C src clean
  # The Linux makefile does not list the Windows-only objects, so a previous
  # Windows build can otherwise leave CDirectDraw.o/CDirectInput.o and, more
  # importantly, objects compiled with -DWIN32. Always start from a
  # platform-clean object tree.
  find src -type f \( \
    -name '*.o' -o \
    -name '*.a' -o \
    -name '*.so' -o \
    -name '*.so.*' -o \
    -name '.depend' -o \
    -name 'Bombermaaan' \
  \) -delete
  make -C src -j"$jobs"

  local package="$BUILD_DIR/Bombermaaan-linux"
  rm -rf "$package"
  mkdir -p "$package/bin" "$package/lib" "$package/Levels" \
    "$package/share/games/bombermaaan/Levels"
  cp src/Bombermaaan/Bombermaaan "$package/bin/"
  cp src/RESGEN/libbombermaaan.so.1.0.0 "$package/lib/"
  ln -s libbombermaaan.so.1.0.0 "$package/lib/libbombermaaan.so.1"
  ln -s libbombermaaan.so.1.0.0 "$package/lib/libbombermaaan.so"

  local levels_dir="$PROJECT_ROOT/levels"
  find "$levels_dir" -maxdepth 1 -type f -iname '*.txt' \
    -exec cp {} "$package/Levels/" \;
  find "$package/Levels" -maxdepth 1 -type f \
    -exec cp {} "$package/share/games/bombermaaan/Levels/" \;
  cp "$PROJECT_ROOT/src/debian/usr/share/doc/bombermaaan/Readme.html" "$package/"
  cp "$PROJECT_ROOT/COPYING.txt" "$package/"
  cp "$PROJECT_ROOT/src/CHANGELOG.txt" "$package/"
  printf '%s\n' '#!/bin/sh' \
    'ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"' \
    'cd "$ROOT" || exit 1' \
    'LD_LIBRARY_PATH="$ROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" exec "$ROOT/bin/Bombermaaan" "$@"' \
    > "$package/launch.sh"
  chmod +x "$package/launch.sh"
  (cd "$BUILD_DIR" && zip -qr Bombermaaan-linux.zip Bombermaaan-linux)
}

if [[ "${IN_CONTAINER:-0}" == "1" ]]; then
  build_linux
  exit 0
fi

if [[ -n "${CONTAINER_ENGINE:-}" ]]; then
  ENGINE="$CONTAINER_ENGINE"
elif command -v podman >/dev/null 2>&1; then
  ENGINE="podman"
elif command -v docker >/dev/null 2>&1; then
  ENGINE="docker"
else
  echo "[ERROR] Neither podman nor docker was found." >&2
  exit 2
fi

if [[ "${1:-}" == "clean-image" ]]; then
  "$ENGINE" image rm "$IMAGE" || true
  exit 0
fi

if [[ "${IMAGE_PULL:-0}" == "1" ]]; then
  "$ENGINE" pull "$IMAGE"
fi
if ! "$ENGINE" image inspect "$IMAGE" >/dev/null 2>&1; then
  echo "Building Linux builder image: $IMAGE"
  "$ENGINE" build --tag "$IMAGE" \
    --file "$PROJECT_ROOT/docker/Dockerfile.linux" "$PROJECT_ROOT"
fi

VOLUME_SUFFIX="${CONTAINER_VOLUME_SUFFIX:-}"
if [[ -z "$VOLUME_SUFFIX" && "$(basename "$ENGINE")" == podman* ]]; then
  VOLUME_SUFFIX=":Z"
fi

USER_ARGS=()
if [[ "${CONTAINER_USE_HOST_USER:-1}" == "1" ]]; then
  if [[ "$(basename "$ENGINE")" == podman* ]]; then
    USER_ARGS+=(--userns=keep-id)
  else
    USER_ARGS+=(--user "$(id -u):$(id -g)")
  fi
fi

TTY_ARGS=()
if [[ -t 0 && -t 1 ]]; then
  TTY_ARGS+=(-it)
fi

exec "$ENGINE" run --rm "${TTY_ARGS[@]}" "${USER_ARGS[@]}" \
  -e BUILD_DIR="$BUILD_DIR" \
  -e JOBS="${JOBS:-}" \
  -e IN_CONTAINER=1 \
  -v "$PROJECT_ROOT:/workspace${VOLUME_SUFFIX}" \
  -w /workspace \
  "$IMAGE" bash -lc './scripts/docker-build-linux.sh'
