# 🔬 windows-dll-loading-rnd

[![Windows x64 Cross-Build & PE Inspection](https://github.com/outspoken/windows-dll-loading-rnd/actions/workflows/build.yml/badge.svg)](.github/workflows/build.yml)
![Platform](https://img.shields.io/badge/Target-Windows%20x86__64%20(PE32%2B)-0078D6)
![Toolchain](https://img.shields.io/badge/Toolchain-LLVM--MinGW%20%7C%20Clang%20%7C%20NASM%20%7C%20LLD-blue)
![Standard](https://img.shields.io/badge/C-C11%20Strict%20(-Werror)-success)

> **Defensive Windows Internals & PE32+ Loader Research Laboratory**  
> Reproducible cross-compilation environment (Fedora Linux $\to$ Windows x86-64) for studying dynamic library resolution, `LoadLibraryExW` search-order hardening, PE/COFF data directories, and the Microsoft x64 Calling Convention (Win64 ABI).

---

## 🎯 Research Scope & Components (`v0.1`)

| Component | Binary Output | Technical Focus |
| :--- | :--- | :--- |
| **`benign_dll`** | `build/benign.dll` | Loader-lock compliant `DllMain` (`DisableThreadLibraryCalls`) and clean PE Export Directory controlled via an explicit module-definition file (`benign.def`). |
| **`safe_loader`** | `build/safe_loader_demo.exe` | Hardened dynamic loading via `SetDefaultDllDirectories` and `LoadLibraryExW(..., LOAD_LIBRARY_SEARCH_APPLICATION_DIR)` to eliminate insecure CWD/PATH resolution. |
| **`abi_demo`** | `build/abi_demo.exe` | C11 $\leftrightarrow$ NASM Win64 COFF interoperability validating `RCX, RDX, R8, R9` register arguments, caller shadow space (`[RSP+0x28]`), and non-volatile register preservation (`RBX, RDI, RSI, R12, R13`). |
| **`pe_report`** | `build/pe_report.exe` | Zero-dependency C11 static PE32+ parser translating Relative Virtual Addresses (RVAs) across section headers to inspect Import and Export Directories. |

---

## 🚀 Quick Start (Fedora Linux Host)

```bash
# 1. Install host packages & pinned LLVM-MinGW (20240619)
chmod +x scripts/*.sh
./scripts/bootstrap-fedora.sh
source ./scripts/env.sh

# 2. Verify compiler, assembler, and LLVM binary utilities
make check-tools

# 3. Compile all PE32+ binaries with strict C11 flags (-Wall -Wextra -Werror -Wpedantic)
make clean all

# 4. Generate PE32+ header & Import/Export telemetry reports
make inspect
```

### Build & Inspection Output
```text
build/
├── abi_demo.exe
├── abi_win64.obj
├── benign.dll
├── libbenign.dll.a
├── pe_report.exe
├── safe_loader_demo.exe
└── reports/
    ├── abi_demo.exe.txt
    ├── benign.dll.txt
    ├── pe_report.exe.txt
    └── safe_loader_demo.exe.txt
```

---

## 📚 Documentation

- [Architecture & Windows Internals Reference](docs/ARCHITECTURE.md)
- [Cross-Compilation & Build Guide](docs/BUILD.md)
- [Binary Validation & PE Telemetry Guide](docs/VALIDATION.md)
- [Technical Specification (v0.1)](SPECIFICATION.md)
- [Disclaimer & Legal Scope](DISCLAIMER.md)
- [Security Policy](SECURITY.md)

---

## 🗺️ R&D Roadmap

| Milestone | Deliverable | Status |
| :--- | :--- | :--- |
| **v0.1** | Cross-build pipeline, hardened `LoadLibraryExW`, Win64 ABI NASM bridge, C11 PE32+ inspector | ✅ Complete |
| **v0.2** | Extended PE32+ parser: Bound Imports, Delay-Load Directory, and Rich Header analysis | ⏳ Planned |
| **v0.3** | Controlled DLL Search Order telemetry harness (`Knowndlls`, `.local` redirection, SxS manifests) | ⏳ Planned |
| **v0.4** | Export forwarding chains & API Set Schema (`api-ms-win-*`) resolution study | ⏳ Planned |
| **v0.5** | Detection Engineering: Sysmon Event ID 7 (`ImageLoaded`) & ETW `Microsoft-Windows-Kernel-Audit-API-Calls` rules | ⏳ Planned |
