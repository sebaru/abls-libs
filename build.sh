#!/bin/bash
# build.sh — Compile abls-libs dans le répertoire build/
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building abls-libs..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory:   $BUILD_DIR"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake ..
cmake --build . -- -j$(nproc)

echo ""
echo "Build completed. Install with: sudo ./install.sh"
