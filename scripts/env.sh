#!/usr/bin/env bash
# =============================================================================
# env.sh — Adds pinned LLVM-MinGW toolchain to PATH for the current shell
# Usage: source ./scripts/env.sh
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [ -d "${REPO_ROOT}/.toolchain/llvm-mingw/bin" ]; then
    LLVM_MINGW_BIN="${REPO_ROOT}/.toolchain/llvm-mingw/bin"
elif [ -d "${HOME}/.local/opt/llvm-mingw/bin" ]; then
    LLVM_MINGW_BIN="${HOME}/.local/opt/llvm-mingw/bin"
else
    LLVM_MINGW_BIN="${HOME}/.local/opt/llvm-mingw/bin"
fi

if [ -d "${LLVM_MINGW_BIN}" ]; then
    case ":${PATH}:" in
        *":${LLVM_MINGW_BIN}:"*) ;;
        *) export PATH="${LLVM_MINGW_BIN}:${PATH}" ;;
    esac
    echo "[+] Added ${LLVM_MINGW_BIN} to PATH"
else
    echo "[!] Warning: ${LLVM_MINGW_BIN} not found. Run ./scripts/bootstrap-fedora.sh first."
fi
