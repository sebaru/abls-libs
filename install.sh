#!/bin/bash
# install.sh — Installe abls-habitat-libs sur le système
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

if [ ! -d "$BUILD_DIR" ]; then
  echo "Build directory not found. Run ./build.sh first."
  exit 1
fi

cmake --install "$BUILD_DIR"
ldconfig
echo "Installation complete."
