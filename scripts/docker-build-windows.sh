#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE="${WINDOWS_DOCKER_IMAGE:-bombermaaan-windows-builder:latest}"
BUILD_DIR="${BUILD_DIR:-build-windows}"
LOG_FILE="${WINDOWS_BUILD_LOG:-$PROJECT_DIR/docker-build-windows.log}"

build_windows() {
  local jobs="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"
  : "${MINGW_PREFIX:=/mingw64}"
  : "${CXX:=x86_64-w64-mingw32-g++}"
  : "${CC:=x86_64-w64-mingw32-gcc}"
  if [[ -z "${WINDRES:-}" ]]; then
    if command -v windres >/dev/null 2>&1; then
      WINDRES=windres
    else
      WINDRES=x86_64-w64-mingw32-windres
    fi
  fi
  : "${SDL_CONFIG:=$MINGW_PREFIX/bin/sdl-config}"
  : "${DIRECTX_INC:=../third-party/DXSDK_Aug2007/Include}"
  : "${DIRECTX_LIB:=../third-party/DXSDK_Aug2007/Lib/x64}"

  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  # Never reuse objects produced by the Linux build (or by a different
  # MinGW target). The two builds use different preprocessor definitions and
  # object formats; mixing them can make the linker crash instead of reporting
  # a normal error.
  find src/RES src/RES32 src/Bombermaaan -type f \( \
    -name '*.o' -o \
    -name '*.a' -o \
    -name '*.res' -o \
    -name '.depend' -o \
    -name 'Bombermaaan_*.exe' -o \
    -name 'Bombermaaan*.dll' \
  \) -delete
  make -C src/RES -f Makefile.win CROSS=1 CPP="$CXX" CC="$CC" WINDRES="$WINDRES"
  make -C src/RES32 -f Makefile.win CROSS=1 CPP="$CXX" CC="$CC" WINDRES="$WINDRES"
  make -B -C src/third-party/tinyxml \
    CXX="$CXX" CXXFLAGS="-O2 -Wno-format -DCROSS"
  make -B -C src/Bombermaaan -f Makefile.win CROSS=1 \
    CPP="$CXX" CC="$CC" WINDRES="$WINDRES" SDL_CONFIG="$SDL_CONFIG" \
    DIRECTX_INC="$DIRECTX_INC" DIRECTX_LIB="$DIRECTX_LIB"

  local package="$BUILD_DIR/Bombermaaan-windows"
  rm -rf "$package"
  mkdir -p "$package/Levels"
  cp levels/*.TXT "$package/Levels/"
  cp src/debian/usr/share/doc/bombermaaan/Readme.html "$package/"
  cp COPYING.txt "$package/"
  cp src/CHANGELOG.txt "$package/"
  cp src/Bombermaaan/Bombermaaan_32.exe "$package/Bombermaaan.exe"
  rm -f "$package"/*.dll
  cp src/RES/Bombermaaan.dll src/RES32/Bombermaaan_32.dll "$package/"

  local queue=(Bombermaaan.exe Bombermaaan.dll Bombermaaan_32.dll)
  declare -A seen=()
  while ((${#queue[@]})); do
    local file="${queue[0]}"
    queue=("${queue[@]:1}")
    while read -r dependency; do
      [[ -n "$dependency" && -f "$dependency" ]] || continue
      local name="$(basename "$dependency")"
      [[ -n "${seen[$name]:-}" ]] && continue
      seen["$name"]=1
      cp "$dependency" "$package/$name"
      queue+=("$name")
    done < <(ldd "$package/$file" 2>/dev/null |
      sed -n "s|.*=> \($MINGW_PREFIX/bin/[^ ]*\.dll\).*|\1|p")
  done

  (cd "$BUILD_DIR" && zip -qr Bombermaaan-windows.zip Bombermaaan-windows)
}

# Run the build inside the project-specific Windows builder image.
if [[ "${IN_CONTAINER:-0}" == "1" ]]; then
  cd "$PROJECT_DIR"
  build_windows
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
  echo "Building Windows builder image: $IMAGE"
  "$ENGINE" build --tag "$IMAGE" \
    --file "$PROJECT_DIR/docker/Dockerfile.windows" "$PROJECT_DIR"
fi

VOLUME_SUFFIX="${CONTAINER_VOLUME_SUFFIX:-}"
if [[ -z "$VOLUME_SUFFIX" && "$(basename "$ENGINE")" == podman* ]]; then
  VOLUME_SUFFIX=":Z"
fi

TTY_ARGS=()
if [[ -t 0 && -t 1 ]]; then
  TTY_ARGS+=(-it)
fi

set +e
"$ENGINE" run --rm "${TTY_ARGS[@]}" \
  -e BUILD_DIR="$BUILD_DIR" \
  -e JOBS="${JOBS:-}" \
  -e IN_CONTAINER=1 \
  -e MINGW_PREFIX=/opt/mingw \
  -e SDL_CONFIG=/opt/mingw/bin/sdl-config \
  -v "$PROJECT_DIR:/workspace${VOLUME_SUFFIX}" \
  -w /workspace \
  "$IMAGE" bash -lc '
    set -euo pipefail
    command -v make >/dev/null 2>&1 || {
      apt-get update
      DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends make zip
      rm -rf /var/lib/apt/lists/*
    }
    bash ./scripts/docker-build-windows.sh
  ' 2>&1 | tee "$LOG_FILE"
status="${PIPESTATUS[0]}"
set -e
exit "$status"
