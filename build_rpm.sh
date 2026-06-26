#!/bin/bash
# build_rpm.sh — Fabrique les RPMs via CPack
set -euo pipefail

PACKAGE_ONLY=false

case "${1:-}" in
  "")
    ;;
  --package-only|-p)
    PACKAGE_ONLY=true
    ;;
  *)
    echo "Usage: $0 [--package-only|-p]"
    exit 2
    ;;
esac

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building RPM packages for abls-libs..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory:   $BUILD_DIR"
echo "Package-only mode: $PACKAGE_ONLY"

mkdir -p "$BUILD_DIR"

if [[ "$PACKAGE_ONLY" == "false" ]]; then
  cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"
  cmake --build "$BUILD_DIR" -- -j"$(nproc)"
elif [[ ! -f "$BUILD_DIR/CPackConfig.cmake" ]]; then
  echo "Missing $BUILD_DIR/CPackConfig.cmake. Run ./build.sh first or use full mode."
  exit 1
fi

rm -f "$BUILD_DIR"/abls-libs-*.rpm "$BUILD_DIR"/abls-libs-devel-*.rpm

pushd "$BUILD_DIR" >/dev/null
cpack -G RPM
popd >/dev/null

runtime_rpm=$(find "$BUILD_DIR" -maxdepth 1 -type f -name 'abls-libs-[0-9]*.rpm' | sort | tail -n 1)
devel_rpm=$(find "$BUILD_DIR" -maxdepth 1 -type f -name 'abls-libs-devel-[0-9]*.rpm' | sort | tail -n 1)

if [[ -z "$runtime_rpm" || -z "$devel_rpm" ]]; then
  echo "RPM generation failed: expected runtime and devel packages in $BUILD_DIR"
  exit 1
fi

echo "RPMs generated:"
echo "  $runtime_rpm"
echo "  $devel_rpm"
echo "RPM build complete."