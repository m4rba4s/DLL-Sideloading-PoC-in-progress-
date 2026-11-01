# ⚠️ Legal & Educational Disclaimer

This repository (`windows-dll-loading-rnd`) is developed strictly for **educational, academic, and defensive security research purposes** focusing on Windows Internals, Portable Executable (PE32+) binary structures, and secure dynamic library loading practices.

1. **No Offensive Payloads**: This repository contains only benign, self-contained C11 and x86-64 assembly demonstrations (`benign.dll`, `safe_loader_demo.exe`, `abi_demo.exe`, `pe_report.exe`). It does not include exploit payloads, shellcode loaders, or third-party vendor binaries.
2. **Defensive Focus**: All examples are designed to help software engineers, reverse engineers, and detection engineers understand how the Windows Loader (`ntdll!LdrLoadDll`) resolves modules and how to harden applications using `SetDefaultDllDirectories` and `LoadLibraryExW`.
3. **Authorized Testing Only**: Any experimentation based on these concepts must be performed exclusively inside isolated laboratory environments owned by the researcher or with explicit written authorization.
