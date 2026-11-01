#ifndef BENIGN_API_H
#define BENIGN_API_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns the semantic version of benign.dll encoded as 0xMMmmpppp
 * (e.g., 0x00010000 for v0.1.0).
 */
typedef uint32_t (__cdecl *pfn_BenignGetVersion)(void);

/*
 * Computes a deterministic 32-bit FNV-1a checksum over a byte buffer.
 */
typedef uint32_t (__cdecl *pfn_BenignComputeChecksum)(const uint8_t *data, size_t len);

/*
 * Formats process/thread/module telemetry into a caller-allocated UTF-16 buffer.
 * Returns the number of wide characters written (excluding null terminator), or -1 on error.
 */
typedef int (__cdecl *pfn_BenignFormatTelemetry)(wchar_t *buffer, size_t cch_buffer);

uint32_t __cdecl BenignGetVersion(void);
uint32_t __cdecl BenignComputeChecksum(const uint8_t *data, size_t len);
int __cdecl BenignFormatTelemetry(wchar_t *buffer, size_t cch_buffer);

#ifdef __cplusplus
}
#endif

#endif /* BENIGN_API_H */
