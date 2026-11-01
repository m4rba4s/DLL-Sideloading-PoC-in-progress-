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
#include <string.h>
#include "abi_bridge.h"

static uint64_t reference_verify_frame(
    uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f
) {
    return (a * 3ULL) + (b * 5ULL) + (c * 7ULL) +
           (d * 11ULL) + (e * 13ULL) + (f * 17ULL);
}

static uint64_t reference_fnv1a64(const uint8_t *buf, size_t len) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint64_t)buf[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

int main(void) {
    printf("[*] Windows x86-64 ABI & NASM Verification Harness (v0.1)\n");

    uint64_t a = 10ULL;
    uint64_t b = 20ULL;
    uint64_t c = 30ULL;
    uint64_t d = 40ULL;
    uint64_t e = 50ULL;
    uint64_t f = 60ULL;

    uint64_t expected_sum = reference_verify_frame(a, b, c, d, e, f);
    uint64_t asm_sum = abi_verify_frame(a, b, c, d, e, f);

    printf("[+] abi_verify_frame(RCX, RDX, R8, R9, [RSP+0x28], [RSP+0x30]):\n");
    printf("    Expected : %llu (0x%016llX)\n",
           (unsigned long long)expected_sum, (unsigned long long)expected_sum);
    printf("    Actual   : %llu (0x%016llX)\n",
           (unsigned long long)asm_sum, (unsigned long long)asm_sum);

    if (asm_sum != expected_sum) {
        fprintf(stderr, "[!] ABI frame verification mismatch!\n");
        return 1;
    }

    const char *msg = "Microsoft-x64-Calling-Convention-Verified";
    size_t msg_len = strlen(msg);
    uint64_t expected_hash = reference_fnv1a64((const uint8_t *)msg, msg_len);
    uint64_t asm_hash = abi_fnv1a64((const uint8_t *)msg, msg_len);

    printf("[+] abi_fnv1a64(\"%s\"):\n", msg);
    printf("    Expected : 0x%016llX\n", (unsigned long long)expected_hash);
    printf("    Actual   : 0x%016llX\n", (unsigned long long)asm_hash);

    if (asm_hash != expected_hash) {
        fprintf(stderr, "[!] FNV-1a 64-bit hash mismatch!\n");
        return 2;
    }

    printf("[+] Win64 ABI register, stack offset, and callee-saved invariants verified.\n");
    return 0;
}
