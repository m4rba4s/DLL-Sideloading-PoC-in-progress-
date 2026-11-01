#ifndef ABI_BRIDGE_H
#define ABI_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Microsoft x64 Calling Convention (Win64 ABI) verification routine.
 * Implemented in NASM (src/abi_demo/abi_win64.asm).
 *
 * Parameter mapping per Win64 ABI:
 *   a_rcx    -> RCX
 *   b_rdx    -> RDX
 *   c_r8     -> R8
 *   d_r9     -> R9
 *   e_stack5 -> [RSP + 0x28] (after 8-byte return address + 32-byte shadow space)
 *   f_stack6 -> [RSP + 0x30]
 *
 * Computes:
 *   result = (a_rcx * 3) + (b_rdx * 5) + (c_r8 * 7) + (d_r9 * 11) + (e_stack5 * 13) + (f_stack6 * 17)
 * while exercising and preserving non-volatile registers RBX, RDI, RSI, R12, and R13.
 */
uint64_t abi_verify_frame(
    uint64_t a_rcx,
    uint64_t b_rdx,
    uint64_t c_r8,
    uint64_t d_r9,
    uint64_t e_stack5,
    uint64_t f_stack6
);

/*
 * Computes a 64-bit FNV-1a hash in pure x86-64 assembly following Win64 ABI.
 *   buf -> RCX
 *   len -> RDX
 *   returns hash in RAX.
 */
uint64_t abi_fnv1a64(const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* ABI_BRIDGE_H */
