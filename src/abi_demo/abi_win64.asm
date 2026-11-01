; =============================================================================
; File: src/abi_demo/abi_win64.asm
; Target: Windows x86-64 COFF (nasm -f win64)
; Calling Convention: Microsoft x64 ABI
;   - Integer/Pointer Args 1..4 : RCX, RDX, R8, R9
;   - Args 5..6                 : [RSP + 0x28], [RSP + 0x30] (at function entry)
;   - Return Value              : RAX
;   - Non-Volatile Registers    : RBX, RBP, RDI, RSI, RSP, R12..R15, XMM6..XMM15
; =============================================================================

bits 64
default rel

section .text

global abi_verify_frame
global abi_fnv1a64

; -----------------------------------------------------------------------------
; uint64_t abi_verify_frame(
;     uint64_t a_rcx,     // RCX
;     uint64_t b_rdx,     // RDX
;     uint64_t c_r8,      // R8
;     uint64_t d_r9,      // R9
;     uint64_t e_stack5,  // [RSP + 0x28] on entry
;     uint64_t f_stack6   // [RSP + 0x30] on entry
; );
; -----------------------------------------------------------------------------
abi_verify_frame:
    ; Save non-volatile registers that we intentionally exercise (RBX, RDI, RSI, R12, R13)
    ; 5 pushes = 40 bytes (0x28), shifting original RSP offsets by +0x28
    push rbx
    push rdi
    push rsi
    push r12
    push r13

    ; Load stack arguments (originally at [rsp + 0x28] and [rsp + 0x30],
    ; now shifted by 5 * 8 = 0x28 bytes -> [rsp + 0x50] and [rsp + 0x58])
    mov r12, [rsp + 0x50]       ; e_stack5
    mov r13, [rsp + 0x58]       ; f_stack6

    ; Compute weighted sum using non-volatile registers to prove preservation:
    ; result = a*3 + b*5 + c*7 + d*11 + e*13 + f*17
    imul rax, rcx, 3            ; a_rcx * 3
    imul rbx, rdx, 5            ; b_rdx * 5
    imul rdi, r8,  7            ; c_r8  * 7
    imul rsi, r9,  11           ; d_r9  * 11
    imul r12, r12, 13           ; e_stack5 * 13
    imul r13, r13, 17           ; f_stack6 * 17

    add rax, rbx
    add rax, rdi
    add rax, rsi
    add rax, r12
    add rax, r13

    ; Restore non-volatile registers in reverse order
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbx
    ret

; -----------------------------------------------------------------------------
; uint64_t abi_fnv1a64(const uint8_t *buf, size_t len);
;   RCX = buf
;   RDX = len
;   RAX = 64-bit FNV-1a digest
; -----------------------------------------------------------------------------
abi_fnv1a64:
    mov rax, 0xcbf29ce484222325 ; FNV-1a 64-bit offset basis
    test rcx, rcx
    jz .done
    test rdx, rdx
    jz .done

    mov r8, 0x100000001b3       ; FNV-1a 64-bit prime

.loop:
    movzx r9d, byte [rcx]
    xor rax, r9
    imul rax, r8
    inc rcx
    dec rdx
    jnz .loop

.done:
    ret
