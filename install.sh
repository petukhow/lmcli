#!/usr/bin/env bash
# Builds and installs lmcli from source.
# Usage: curl -fsSL https://raw.githubusercontent.com/petukhow/lmcli/master/install.sh | bash
set -euo pipefail

REPO_URL="https://github.com/petukhow/lmcli.git"
INSTALL_PREFIX="${LMCLI_INSTALL_PREFIX:-/usr/local}"
WORK_DIR="$(mktemp -d)"

cleanup() { rm -rf "$WORK_DIR"; }
trap cleanup EXIT

echo "==> Checking build prerequisites"

missing=()
command -v git   >/dev/null 2>&1 || missing+=("git")
command -v cmake >/dev/null 2>&1 || missing+=("cmake (3.15+)")
command -v g++   >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || missing+=("a C++17 compiler (g++ or clang++)")

if command -v pkg-config >/dev/null 2>&1; then
    pkg-config --exists libcurl 2>/dev/null || missing+=("libcurl development headers (e.g. libcurl4-openssl-dev, libcurl-devel)")
fi

if [ "${#missing[@]}" -ne 0 ]; then
    echo "Missing required dependencies:"
    for dep in "${missing[@]}"; do
        echo "  - $dep"
    done
    echo
    echo "Install them with your system's package manager, then re-run this script."
    exit 1
fi

echo "==> Cloning lmcli"
git clone --depth 1 "$REPO_URL" "$WORK_DIR/lmcli"

echo "==> Configuring (Release build)"
cmake -B "$WORK_DIR/lmcli/build" -S "$WORK_DIR/lmcli" -DCMAKE_BUILD_TYPE=Release

echo "==> Building"
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"
cmake --build "$WORK_DIR/lmcli/build" -j"$JOBS"

echo "==> Installing to $INSTALL_PREFIX (requires sudo)"
sudo cmake --install "$WORK_DIR/lmcli/build" --prefix "$INSTALL_PREFIX"

echo
echo "lmcli installed to $INSTALL_PREFIX/bin/lmcli"
echo "Run 'lmcli' to get started."
