#!/usr/bin/env bash
# =============================================================================
# bootstrap-fedora.sh — Reproducible Toolchain Setup for Fedora Linux
# Installs host build dependencies and pinned LLVM-MinGW (ucrt / x86_64)
# =============================================================================
set -euo pipefail

LLVM_MINGW_VERSION="${LLVM_MINGW_VERSION:-20240619}"
LLVM_MINGW_ARCH="ucrt-ubuntu-20.04-x86_64"
LLVM_MINGW_TARBALL="llvm-mingw-${LLVM_MINGW_VERSION}-${LLVM_MINGW_ARCH}.tar.xz"
LLVM_MINGW_URL="https://github.com/mstorsjo/llvm-mingw/releases/download/${LLVM_MINGW_VERSION}/${LLVM_MINGW_TARBALL}"
INSTALL_PREFIX="${HOME}/.local/opt/llvm-mingw"

echo "[*] Step 1: Checking Fedora host packages..."
REQUIRED_PKGS=(nasm llvm clang lld make git curl jq xz file)
MISSING_PKGS=()

for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! rpm -q "$pkg" >/dev/null 2>&1; then
        MISSING_PKGS+=("$pkg")
    fi
done

if [ "${#MISSING_PKGS[@]}" -gt 0 ]; then
    echo "[*] Installing missing Fedora packages: ${MISSING_PKGS[*]}"
    sudo dnf install -y "${MISSING_PKGS[@]}"
else
    echo "[+] All required Fedora host packages are already installed."
fi

echo "[*] Step 2: Checking LLVM-MinGW installation at ${INSTALL_PREFIX}..."
if [ -x "${INSTALL_PREFIX}/bin/x86_64-w64-mingw32-clang" ]; then
    echo "[+] LLVM-MinGW is already installed at ${INSTALL_PREFIX}"
    "${INSTALL_PREFIX}/bin/x86_64-w64-mingw32-clang" --version | head -n 1
    exit 0
fi

mkdir -p "${HOME}/.local/opt"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

echo "[*] Downloading pinned LLVM-MinGW (${LLVM_MINGW_VERSION})..."
echo "    URL: ${LLVM_MINGW_URL}"
curl -fL --retry 3 -o "${TMP_DIR}/${LLVM_MINGW_TARBALL}" "${LLVM_MINGW_URL}"

echo "[*] Extracting toolchain to ${INSTALL_PREFIX}..."
tar -xJf "${TMP_DIR}/${LLVM_MINGW_TARBALL}" -C "${TMP_DIR}"
rm -rf "${INSTALL_PREFIX}"
mv "${TMP_DIR}/llvm-mingw-${LLVM_MINGW_VERSION}-${LLVM_MINGW_ARCH}" "${INSTALL_PREFIX}"

echo "[+] LLVM-MinGW installed successfully!"
echo "[+] Run: source ./scripts/env.sh"
