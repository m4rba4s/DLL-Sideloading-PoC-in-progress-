# =============================================================================
# windows-dll-loading-rnd — Canonical Cross-Compilation Makefile (v0.1)
# Host: Fedora Linux (x86_64) -> Target: Windows x86-64 (PE32+)
# =============================================================================

SHELL := /bin/bash
.ONESHELL:
.SUFFIXES:

# Override GNU Make's built-in default (CC = cc)
ifeq ($(origin CC), default)
    ifneq ($(shell command -v x86_64-w64-mingw32-clang 2>/dev/null),)
        CC := x86_64-w64-mingw32-clang
        LDFLAGS_EXTRA := -fuse-ld=lld
    else ifneq ($(shell command -v x86_64-w64-mingw32-gcc 2>/dev/null),)
        CC := x86_64-w64-mingw32-gcc
        LDFLAGS_EXTRA :=
    else
        CC := x86_64-w64-mingw32-clang
        LDFLAGS_EXTRA := -fuse-ld=lld
    endif
else
    LDFLAGS_EXTRA ?= -fuse-ld=lld
endif

NASM        ?= nasm
READOBJ     ?= llvm-readobj
FILE_CMD    ?= file

BUILD_DIR   := build
REPORT_DIR  := $(BUILD_DIR)/reports
INC_DIR     := include

CFLAGS      := -std=c11 -O2 -Wall -Wextra -Werror -Wpedantic \
               -DWIN32_LEAN_AND_MEAN -DUNICODE -D_UNICODE \
               -I$(INC_DIR)

NASMFLAGS   := -f win64 -Werror

LDFLAGS     := $(LDFLAGS_EXTRA) \
               -Wl,--nxcompat \
               -Wl,--dynamicbase \
               -Wl,--high-entropy-va

# Targets
BENIGN_DLL  := $(BUILD_DIR)/benign.dll
SAFE_LOADER := $(BUILD_DIR)/safe_loader_demo.exe
ABI_DEMO    := $(BUILD_DIR)/abi_demo.exe
PE_REPORT   := $(BUILD_DIR)/pe_report.exe
ABI_OBJ     := $(BUILD_DIR)/abi_win64.obj

ALL_BINS    := $(BENIGN_DLL) $(SAFE_LOADER) $(ABI_DEMO) $(PE_REPORT)

.PHONY: all clean check-tools inspect

all: $(ALL_BINS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(REPORT_DIR):
	mkdir -p $(REPORT_DIR)

check-tools:
	@echo "[*] Checking cross-compilation and inspection toolchain..."
	@command -v $(firstword $(CC)) >/dev/null 2>&1 || { \
		echo "[!] Missing C cross-compiler ($(CC)). Run ./scripts/bootstrap-fedora.sh and source ./scripts/env.sh"; \
		exit 1; \
	}
	@command -v $(NASM) >/dev/null 2>&1 || { echo "[!] Missing $(NASM)"; exit 1; }
	@command -v $(READOBJ) >/dev/null 2>&1 || { echo "[!] Missing $(READOBJ)"; exit 1; }
	@command -v $(FILE_CMD) >/dev/null 2>&1 || { echo "[!] Missing $(FILE_CMD)"; exit 1; }
	@echo "[+] Toolchain verified:"
	@echo "    CC      = $(CC)"
	@echo "    NASM    = $$($(NASM) -v)"
	@echo "    READOBJ = $$(command -v $(READOBJ))"

$(BENIGN_DLL): src/benign_dll/benign.c src/benign_dll/benign.def $(INC_DIR)/benign_api.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -shared \
		src/benign_dll/benign.c \
		src/benign_dll/benign.def \
		$(LDFLAGS) \
		-Wl,--out-implib,$(BUILD_DIR)/libbenign.dll.a \
		-o $@

$(SAFE_LOADER): src/safe_loader/safe_loader_demo.c $(INC_DIR)/benign_api.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		src/safe_loader/safe_loader_demo.c \
		$(LDFLAGS) \
		-o $@

$(ABI_OBJ): src/abi_demo/abi_win64.asm | $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) src/abi_demo/abi_win64.asm -o $@

$(ABI_DEMO): src/abi_demo/abi_main.c $(ABI_OBJ) $(INC_DIR)/abi_bridge.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		src/abi_demo/abi_main.c \
		$(ABI_OBJ) \
		$(LDFLAGS) \
		-o $@

$(PE_REPORT): src/pe_report/pe_report.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		src/pe_report/pe_report.c \
		$(LDFLAGS) \
		-o $@

inspect: $(ALL_BINS) | $(REPORT_DIR)
	@echo "[*] Generating PE32+ structural inspection reports..."
	@for bin in $(ALL_BINS); do \
		base=$$(basename "$$bin"); \
		report="$(REPORT_DIR)/$${base}.txt"; \
		echo "=== FILE IDENTIFICATION ===" > "$$report"; \
		$(FILE_CMD) "$$bin" >> "$$report"; \
		echo "" >> "$$report"; \
		echo "=== LLVM-READOBJ PE32+ TELEMETRY ===" >> "$$report"; \
		$(READOBJ) --file-headers --sections --coff-imports --coff-exports "$$bin" >> "$$report"; \
		echo "[+] Wrote $$report"; \
	done

clean:
	rm -rf $(BUILD_DIR)
