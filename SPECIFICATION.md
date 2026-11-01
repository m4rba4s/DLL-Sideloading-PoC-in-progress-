# 📐 TECHNICAL SPECIFICATION: `windows-dll-loading-rnd` (v0.1)

## 1. Project Identity & Scope
- **Repository Name**: `windows-dll-loading-rnd`
- **Classification**: Educational & Defensive R&D — Windows PE/COFF Internals, Dynamic Linker Mechanics, and x64 ABI Telemetry.
- **Purpose**: A clean, reproducible cross-compilation laboratory demonstrating:
  1. How the Windows NT Loader resolves and maps DLLs using hardened `LoadLibraryExW` flags (`safe_loader_demo.exe` + `benign.dll`).
  2. How the Microsoft x64 Calling Convention (Win64 ABI) operates at the register and stack-frame level between C11 and NASM (`abi_demo.exe`).
  3. How PE32+ (`IMAGE_NT_HEADERS64`) Import/Export Data Directories are structured and inspected both via static C parsing (`pe_report.exe`) and LLVM binary tooling (`make inspect`).

---

## 2. Canonical Build Environment (KISS / YAGNI)
| Layer | Specification | Notes |
| :--- | :--- | :--- |
| **Host OS** | Fedora Linux (x86_64) | CI also supports Ubuntu 24.04 via GitHub Actions |
| **Target OS / Arch** | Windows x86-64 (`PE32+` / `AMD64`) | Minimum subsystem: Windows 10 / Server 2019 (`0x0A00`) |
| **C Compiler** | `x86_64-w64-mingw32-clang` (LLVM-MinGW) | Pinned upstream release `20240619` |
| **Assembler** | `nasm` (`-f win64`) | Generates Win64 COFF `.obj` |
| **Linker** | `ld.lld` (`-fuse-ld=lld`) | Produces deterministic PE32+ binaries with ASLR/DEP enabled |
| **PE Inspection** | `llvm-readobj`, `llvm-objdump`, `file` | Automated report generation in `build/reports/` |

---

## 3. Repository Architecture

```text
windows-dll-loading-rnd/
├── .github/
│   └── workflows/
│       └── build.yml               # Reproducible CI: build + PE header verification
├── docs/
│   ├── ARCHITECTURE.md             # PE32+ layout, Loader Lock, and Win64 ABI diagrams
│   ├── BUILD.md                    # Fedora & CI cross-compilation guide
│   └── VALIDATION.md               # How to verify exports, imports, and mitigations
├── include/
│   ├── benign_api.h                # Shared C header for benign.dll exports
│   └── abi_bridge.h                # Declarations for NASM x64 routines
├── scripts/
│   ├── bootstrap-fedora.sh         # Installs & verifies packages + LLVM-MinGW
│   └── env.sh                      # Exports PATH for llvm-mingw toolchain
├── src/
│   ├── benign_dll/
│   │   ├── benign.c                # Hardened DllMain + exported research functions
│   │   └── benign.def              # Explicit module definition (clean Export Directory)
│   ├── safe_loader/
│   │   └── safe_loader_demo.c      # Explicit path resolution + LoadLibraryExW flags
│   ├── abi_demo/
│   │   ├── abi_main.c              # C harness verifying register/stack preservation
│   │   └── abi_win64.asm           # NASM Win64 COFF routine (RCX, RDX, R8, R9 + shadow space)
│   └── pe_report/
│       └── pe_report.c             # Lightweight Zero-Dependency PE32+ Header & Export/Import Inspector in C
├── Makefile                        # Single canonical build file
├── README.md                       # Portfolio-grade overview & badges
├── DISCLAIMER.md                   # Legal & educational scope statement
├── SECURITY.md                     # Responsible disclosure & safe research policy
└── CONTRIBUTING.md                 # Code style & PR requirements
```
