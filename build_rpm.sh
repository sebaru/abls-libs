#!/bin/bash
# build_rpm.sh — Fabrique les RPMs via CPack
set -euo pipefail

PACKAGE_ONLY=false

for arg in "$@"; do
  case "$arg" in
    --package-only|-p)
      PACKAGE_ONLY=true
      ;;
    -h|--help)
      echo "Usage: $0 [--package-only|-p]"
      exit 0
      ;;
    *)
      echo "Usage: $0 [--package-only|-p]"
      exit 2
      ;;
  esac
done

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building RPM packages for abls-libs..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory:   $BUILD_DIR"
echo "Package-only mode: $PACKAGE_ONLY"
echo "Signing mode:      disabled (centralized in ABLS-PKGS)"
echo "Install prefix:    /usr (forced for RPM packaging)"

mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo

if [[ "$PACKAGE_ONLY" == "false" ]]; then
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
debuginfo_rpm=$(find "$BUILD_DIR" -maxdepth 1 -type f -name '*debuginfo*.rpm' | sort | tail -n 1)

if [[ -z "$runtime_rpm" || -z "$devel_rpm" ]]; then
  echo "RPM generation failed: expected runtime and devel packages in $BUILD_DIR"
  exit 1
fi

if [[ -z "$debuginfo_rpm" ]]; then
  echo "RPM generation failed: expected debuginfo package in $BUILD_DIR"
  exit 1
fi

echo "RPMs generated:"
echo "  $runtime_rpm"
echo "  $devel_rpm"
echo "  $debuginfo_rpm"
echo "RPM build complete."