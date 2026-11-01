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
#include "benign_api.h"

static HMODULE g_hModule = NULL;

uint32_t __cdecl BenignGetVersion(void) {
    return 0x00010000U; /* v0.1.0 */
}

uint32_t __cdecl BenignComputeChecksum(const uint8_t *data, size_t len) {
    if (data == NULL) {
        return 0U;
    }
    uint32_t hash = 2166136261U; /* FNV-1a 32-bit offset basis */
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint32_t)data[i];
        hash *= 16777619U;       /* FNV-1a 32-bit prime */
    }
    return hash;
}

int __cdecl BenignFormatTelemetry(wchar_t *buffer, size_t cch_buffer) {
    if (buffer == NULL || cch_buffer == 0) {
        return -1;
    }

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    uintptr_t base_addr = (uintptr_t)g_hModule;

    int written = _snwprintf(
        buffer,
        cch_buffer,
        L"[benign.dll] PID=%lu | TID=%lu | Base=0x%016llX | Version=0x%08X",
        (unsigned long)pid,
        (unsigned long)tid,
        (unsigned long long)base_addr,
        BenignGetVersion()
    );

    if (written < 0 || (size_t)written >= cch_buffer) {
        buffer[cch_buffer - 1] = L'\0';
        return -1;
    }

    return written;
}

/*
 * Loader-lock compliant DllMain:
 * - No LoadLibrary/FreeLibrary calls
 * - No thread creation or synchronization primitives
 * - Disables DLL_THREAD_ATTACH / DLL_THREAD_DETACH notifications to reduce overhead
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hModule = (HMODULE)hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
    }

    return TRUE;
}
