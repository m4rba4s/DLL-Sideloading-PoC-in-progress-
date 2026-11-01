# 🏛️ Architecture & Windows Internals Reference

## 1. Overview
`windows-dll-loading-rnd` is a defensive systems-engineering laboratory focused on three pillars of Windows x86-64 internals:
1. **Dynamic Linker Hardening (`LdrLoadDll` / `LoadLibraryExW`)** — Mitigating insecure DLL search order resolution.
2. **Microsoft x64 Calling Convention (Win64 ABI)** — Register allocation, caller-allocated shadow space, and non-volatile register preservation across C11 and NASM boundaries.
3. **PE32+ (`IMAGE_NT_HEADERS64`) Binary Telemetry** — Direct inspection of section headers, RVA-to-file-offset translation, and Import/Export Data Directories.

---

## 2. Hardened Dynamic Loading (`safe_loader_demo.exe` & `benign.dll`)

### 2.1 Standard vs. Hardened Search Order
Legacy calls to `LoadLibraryW(L"target.dll")` invoke the standard Win32 search order, which probes the Current Working Directory (CWD) and `PATH` environment directories when SafeDllSearchMode is bypassed or when the application directory is writable.

In `safe_loader_demo.c`, we enforce two layers of defense:
1. **Process-Wide Policy**:
   ```c
   SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
   ```
   This strips the Current Working Directory (CWD) and `PATH` environment variable from the process-wide loader search list.
2. **Per-Call Explicit Scope**:
   ```c
   LoadLibraryExW(L"benign.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
   ```

### 2.2 Loader-Lock Hygiene in `benign.dll`
Inside `DllMain`, the Windows NT Loader holds the process-wide **Loader Lock** (`LdrpLoaderLock`). Calling functions that acquire other synchronization locks, spawn threads, or load additional modules inside `DllMain` risks classical loader deadlocks.
`benign.c` strictly adheres to Microsoft's Loader Lock guidelines:
- Records `hinstDLL` into a module-scoped static variable on `DLL_PROCESS_ATTACH`.
- Calls `DisableThreadLibraryCalls(hinstDLL)` to suppress `DLL_THREAD_ATTACH` and `DLL_THREAD_DETACH` overhead.
- Defers all telemetry formatting and checksum computation to explicit exported functions (`BenignGetVersion`, `BenignComputeChecksum`, `BenignFormatTelemetry`).

---

## 3. Microsoft x64 Calling Convention (`abi_demo.exe`)

Unlike the System V AMD64 ABI used on Linux (`RDI, RSI, RDX, RCX, R8, R9`), Windows x86-64 uses the **Microsoft x64 ABI**:

```text
+-------------------------------------------------------------------+
| Caller Stack Frame (before CALL instruction)                      |
+-------------------------------------------------------------------+
| [RSP + 0x30]  6th Argument (f_stack6)                             |
| [RSP + 0x28]  5th Argument (e_stack5)                             |
| [RSP + 0x20]  Shadow Space / Home Space for R9  (8 bytes)         |
| [RSP + 0x18]  Shadow Space / Home Space for R8  (8 bytes)         |
| [RSP + 0x10]  Shadow Space / Home Space for RDX (8 bytes)         |
| [RSP + 0x08]  Shadow Space / Home Space for RCX (8 bytes)         |
| [RSP + 0x00]  Return Address (pushed by CALL)                     |
+-------------------------------------------------------------------+
```

Key invariants verified by `src/abi_demo/abi_win64.asm`:
- **Arguments 1–4**: Passed in `RCX`, `RDX`, `R8`, `R9`.
- **Arguments 5–6**: Accessed at `[RSP + 0x28]` and `[RSP + 0x30]` at function entry.
- **Callee-Saved Registers**: `RBX`, `RBP`, `RDI`, `RSI`, `RSP`, `R12`–`R15`, and `XMM6`–`XMM15` are non-volatile. Note in particular that `RDI` and `RSI` are non-volatile on Windows x64 (whereas they are volatile argument registers on Linux x86-64).

---

## 4. PE32+ Data Directory Inspection (`pe_report.exe`)

When inspecting a PE32+ file on disk (unmapped raw layout) as opposed to a memory-mapped module (`SEC_IMAGE`), Relative Virtual Addresses (RVAs) stored in `OptionalHeader.DataDirectory[]` cannot be added directly to the file buffer pointer.

`pe_report.c` performs bounds-checked RVA translation via `rva_to_ptr()`:
1. Iterates `IMAGE_SECTION_HEADER` entries (`IMAGE_FIRST_SECTION(nt)`).
2. Identifies the section `i` where `VirtualAddress <= RVA < VirtualAddress + max(VirtualSize, SizeOfRawData)`.
3. Computes the raw file offset:
   $$\text{FileOffset} = \text{PointerToRawData}_i + (\text{RVA} - \text{VirtualAddress}_i)$$
4. Validates that $\text{FileOffset} < \text{FileSize}$ before dereferencing `IMAGE_EXPORT_DIRECTORY` or `IMAGE_IMPORT_DESCRIPTOR`.
