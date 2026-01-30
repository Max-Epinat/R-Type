#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${BUILD_TYPE:-Debug}"
RUN_TARGET="none"
HOST="127.0.0.1"
PORT="5000"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

usage() {
  cat <<'EOF'
Usage: scripts/bootstrap.sh [--build-type Debug|Release] [--run server|client] [--host IP] [--port PORT]

Defaults:
  BUILD_TYPE=Debug
  RUN_TARGET=none (only build)
  HOST=127.0.0.1 (client only)
  PORT=5000

This script:
  - Checks for required tools (conan, cmake, ninja, a C++ compiler).
  - Runs `conan profile detect --force`.
  - Installs deps to build/<type> with Conan.
  - Configures CMake with matching preset.
  - Builds the chosen configuration.
  - Optionally runs the server or client.
EOF
}

die() { echo "error: $*" >&2; exit 1; }
warn() { echo "warn: $*" >&2; }

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "missing required command: $1"
}

auto_install_cmd() {
  local cmd="$1"
  local pkg="${2:-$1}"
  
  if command -v "$cmd" >/dev/null 2>&1; then
    return 0
  fi
  
  echo "==> Installing $pkg..."
  if command -v apt-get >/dev/null 2>&1; then
    sudo apt-get install -y "$pkg"
  elif command -v pacman >/dev/null 2>&1; then
    sudo pacman -S --noconfirm "$pkg"
  elif command -v dnf >/dev/null 2>&1; then
    sudo dnf install -y "$pkg"
  else
    die "Could not auto-install $pkg. Please install it manually."
  fi
}

suggest_conan_install() {
  warn "Conan not found. Install suggestions:"
  if command -v apt-get >/dev/null 2>&1; then
    echo "  sudo apt-get install -y pipx && pipx install conan && pipx ensurepath"
  elif command -v pacman >/dev/null 2>&1; then
    echo "  sudo pacman -S python-pipx && pipx install conan && pipx ensurepath"
  elif command -v dnf >/dev/null 2>&1; then
    echo "  sudo dnf install -y python3-pipx && pipx install conan && pipx ensurepath"
  else
    echo "  pipx install conan  # or: pip install --user conan"
  fi
  echo "  Then restart your terminal or run: source ~/.bashrc"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-type) BUILD_TYPE="$2"; shift 2 ;;
    --run) RUN_TARGET="$2"; shift 2 ;;
    --host) HOST="$2"; shift 2 ;;
    --port) PORT="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) usage; die "unknown argument: $1" ;;
  esac
done

case "$BUILD_TYPE" in
  Debug|Release) ;;
  *) die "BUILD_TYPE must be Debug or Release (got: $BUILD_TYPE)" ;;
esac

if [[ "$RUN_TARGET" != "none" && "$RUN_TARGET" != "server" && "$RUN_TARGET" != "client" ]]; then
  die "--run must be one of: none, server, client"
fi

if ! command -v conan >/dev/null 2>&1; then
  suggest_conan_install
  exit 1
fi

auto_install_cmd cmake cmake
auto_install_cmd ninja ninja-build

echo "==> Detecting Conan profile"
conan profile detect --force

build_dir="${PROJECT_ROOT}/build/${BUILD_TYPE}"
bin_dir="${PROJECT_ROOT}/bin/"
preset_name="conan-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

echo "==> Conan install to ${build_dir}"
(cd "${PROJECT_ROOT}" && conan install . \
  --output-folder="${build_dir}" \
  --build=missing \
  -s "build_type=${BUILD_TYPE}" \
  -s "compiler.cppstd=20" \
  -o "openal-soft/*:with_pipewire=False" \
  -o "openal-soft/*:with_pulseaudio=True" \
  -o "openal-soft/*:with_alsa=True" \
  -c tools.build:cxxflags='["-include","cstdint"]' \
  -c tools.cmake.cmaketoolchain:generator=Ninja\
  -c tools.system.package_manager:mode=install \
  -c tools.system.package_manager:sudo=True)

# Conan may generate a CMakeUserPresets.json that duplicates preset names; drop it to avoid conflicts.
rm -f "${PROJECT_ROOT}/CMakeUserPresets.json"

echo "==> CMake configure (${preset_name})"
(cd "${PROJECT_ROOT}" && cmake --preset "${preset_name}")

echo "==> Building (${BUILD_TYPE})"
(cd "${PROJECT_ROOT}" && cmake --build "${build_dir}")

if [ -f "${build_dir}/compile_commands.json" ]; then
  ln -sf "${build_dir}/compile_commands.json" "${PROJECT_ROOT}/compile_commands.json"
fi

case "$RUN_TARGET" in
  server)
    echo "==> Running server on UDP port ${PORT}"
    exec "${build_dir}/rtype_server" "${PORT}"
    ;;
  client)
    echo "==> Running client connecting to ${HOST}:${PORT}"
    exec "${build_dir}/rtype_client" "${HOST}" "${PORT}"
    ;;
  *)
    echo "Build complete. Binaries are in ${bin_dir}."
    ;;
esac
