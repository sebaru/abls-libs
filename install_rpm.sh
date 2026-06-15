#!/bin/bash
# install_rpm.sh — Fabrique les RPMs via CPack puis réinstalle les paquets locaux
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building RPM packages for abls-libs..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory:   $BUILD_DIR"

mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -- -j"$(nproc)"

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

installed_packages=()

if rpm -q abls-libs >/dev/null 2>&1; then
  installed_packages+=(abls-libs)
fi

if rpm -q abls-libs-devel >/dev/null 2>&1; then
  installed_packages+=(abls-libs-devel)
fi

if [[ ${#installed_packages[@]} -gt 0 ]]; then
  echo "Removing installed packages: ${installed_packages[*]}"
  sudo dnf remove -y "${installed_packages[@]}"
fi

echo "Installing RPMs:"
echo "  $runtime_rpm"
echo "  $devel_rpm"
sudo dnf install -y "$runtime_rpm" "$devel_rpm"

echo "RPM installation complete."