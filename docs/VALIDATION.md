# ✅ Binary Validation & PE Telemetry Guide

## 1. Automated PE32+ Inspection (`make inspect`)

Run the inspection target to generate deterministic structural reports for every compiled artifact:

```bash
make inspect
```

This produces:
- `build/reports/benign.dll.txt`
- `build/reports/safe_loader_demo.exe.txt`
- `build/reports/abi_demo.exe.txt`
- `build/reports/pe_report.exe.txt`

---

## 2. What to Verify in the Reports

### 2.1 File Format & Architecture
```bash
file build/*.exe build/*.dll
```
Every binary must report `PE32+ executable (DLL/console) (console) x86-64, for MS Windows`.

### 2.2 Clean Export Directory in `benign.dll`
```bash
llvm-readobj --coff-exports build/benign.dll
```
Verify that **only** the three research symbols declared in `src/benign_dll/benign.def` are exported:
1. `BenignComputeChecksum`
2. `BenignFormatTelemetry`
3. `BenignGetVersion`

### 2.3 Exploit Mitigations in Optional Header
```bash
llvm-readobj --file-headers build/safe_loader_demo.exe
```
Confirm the presence of:
- `IMAGE_DLL_CHARACTERISTICS_HIGH_ENTROPY_VA (0x20)`
- `IMAGE_DLL_CHARACTERISTICS_DYNAMIC_BASE (0x40)`
- `IMAGE_DLL_CHARACTERISTICS_NX_COMPAT (0x100)`

---

## 3. Runtime Validation on Windows 10/11 x64 (or Wine)

Copy the `build/` directory to a Windows x64 test VM (or run via `wine64` on Fedora):

```powershell
.\abi_demo.exe
.\safe_loader_demo.exe
.\pe_report.exe .\benign.dll
```
All three commands should exit with code `0`.
