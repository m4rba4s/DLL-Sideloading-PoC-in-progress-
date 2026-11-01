# 🛠️ Cross-Compilation & Build Guide (Fedora Linux -> Windows x64)

## 1. Prerequisites on Fedora Linux

Install the required host packages and pinned `llvm-mingw` cross-compiler toolchain:

```bash
chmod +x scripts/*.sh
./scripts/bootstrap-fedora.sh
source ./scripts/env.sh
```

Verify that all tools are reachable:

```bash
make check-tools
```

---

## 2. Building the Project

Compile all four targets (`benign.dll`, `safe_loader_demo.exe`, `abi_demo.exe`, and `pe_report.exe`):

```bash
make clean all
```

### Build Outputs
```text
build/
├── abi_demo.exe          # C11 + NASM Win64 ABI verification executable
├── abi_win64.obj         # Assembled Win64 COFF object file
├── benign.dll            # Research DLL with explicit .def Export Directory
├── libbenign.dll.a       # Import library for benign.dll
├── pe_report.exe         # Standalone C11 PE32+ header & directory inspector
└── safe_loader_demo.exe  # Hardened LoadLibraryExW demonstration executable
```

---

## 3. Hardening Flags Applied by Default

All binaries are linked with LLD (`-fuse-ld=lld`) using modern Windows exploit mitigation flags:
- `-Wl,--nxcompat`: Enables Data Execution Prevention (`IMAGE_DLLCHARACTERISTICS_NX_COMPAT`).
- `-Wl,--dynamicbase`: Enables Address Space Layout Randomization (`IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE`).
- `-Wl,--high-entropy-va`: Enables 64-bit High-Entropy ASLR (`IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA`).
