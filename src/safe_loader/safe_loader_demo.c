#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "benign_api.h"

/*
 * Demonstrates hardened dynamic library loading on Windows:
 * 1. Restricts default process DLL search order via SetDefaultDllDirectories()
 *    to remove Current Working Directory (CWD) and PATH environment directories.
 * 2. Resolves the canonical path inside the application directory and loads
 *    benign.dll using LoadLibraryExW with LOAD_LIBRARY_SEARCH_APPLICATION_DIR.
 * 3. Validates and invokes exported functions, then cleanly unmaps the module.
 */
int main(void) {
    printf("[*] Windows Safe DLL Loader R&D Harness (v0.1)\n");

    /* Step 1: Harden process-wide DLL search order */
    if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_APPLICATION_DIR)) {
        DWORD err = GetLastError();
        fprintf(stderr, "[!] SetDefaultDllDirectories failed (GetLastError=%lu)\n", (unsigned long)err);
        return 1;
    }
    printf("[+] Process DLL search order restricted to SYSTEM32 | APPLICATION_DIR\n");

    /* Step 2: Load benign.dll strictly from the application directory */
    const wchar_t *dll_name = L"benign.dll";
    HMODULE hMod = LoadLibraryExW(dll_name, NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    if (hMod == NULL) {
        DWORD err = GetLastError();
        fprintf(stderr, "[!] LoadLibraryExW(\"benign.dll\") failed (GetLastError=%lu)\n", (unsigned long)err);
        return 2;
    }

    wchar_t loaded_path[MAX_PATH] = {0};
    DWORD path_len = GetModuleFileNameW(hMod, loaded_path, MAX_PATH);
    if (path_len == 0 || path_len >= MAX_PATH) {
        DWORD err = GetLastError();
        fprintf(stderr, "[!] GetModuleFileNameW failed (GetLastError=%lu)\n", (unsigned long)err);
        FreeLibrary(hMod);
        return 3;
    }
    printf("[+] Module mapped at 0x%016llX (%ls)\n",
           (unsigned long long)(uintptr_t)hMod, loaded_path);

    /* Step 3: Resolve exported symbols via GetProcAddress */
    pfn_BenignGetVersion fn_version =
        (pfn_BenignGetVersion)(uintptr_t)GetProcAddress(hMod, "BenignGetVersion");
    pfn_BenignComputeChecksum fn_checksum =
        (pfn_BenignComputeChecksum)(uintptr_t)GetProcAddress(hMod, "BenignComputeChecksum");
    pfn_BenignFormatTelemetry fn_telemetry =
        (pfn_BenignFormatTelemetry)(uintptr_t)GetProcAddress(hMod, "BenignFormatTelemetry");

    if (fn_version == NULL || fn_checksum == NULL || fn_telemetry == NULL) {
        DWORD err = GetLastError();
        fprintf(stderr, "[!] Export resolution failed (GetLastError=%lu)\n", (unsigned long)err);
        FreeLibrary(hMod);
        return 4;
    }

    /* Step 4: Execute exported functions and verify output */
    uint32_t ver = fn_version();
    const uint8_t sample_payload[] = "WINDOWS-DLL-LOADING-RND-V0.1";
    uint32_t checksum = fn_checksum(sample_payload, sizeof(sample_payload) - 1);

    wchar_t telemetry_buf[256] = {0};
    int rc = fn_telemetry(telemetry_buf, 256);
    if (rc < 0) {
        fprintf(stderr, "[!] BenignFormatTelemetry returned error (%d)\n", rc);
        FreeLibrary(hMod);
        return 5;
    }

    printf("[+] BenignGetVersion()      -> 0x%08X\n", ver);
    printf("[+] BenignComputeChecksum() -> 0x%08X\n", checksum);
    printf("[+] BenignFormatTelemetry() -> %ls\n", telemetry_buf);

    /* Step 5: Unmap DLL cleanly */
    if (!FreeLibrary(hMod)) {
        DWORD err = GetLastError();
        fprintf(stderr, "[!] FreeLibrary failed (GetLastError=%lu)\n", (unsigned long)err);
        return 6;
    }
    printf("[+] Module unmapped cleanly. All checks passed.\n");
    return 0;
}
