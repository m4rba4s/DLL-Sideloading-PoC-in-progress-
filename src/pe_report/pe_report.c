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

/*
 * Translates a Relative Virtual Address (RVA) to a raw file pointer inside a
 * memory-mapped PE image, performing bounds checking against the file size.
 */
static const uint8_t *rva_to_ptr(
    const uint8_t *base,
    size_t file_size,
    const IMAGE_NT_HEADERS64 *nt,
    DWORD rva
) {
    if (rva == 0) {
        return NULL;
    }

    const IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
    WORD num_sections = nt->FileHeader.NumberOfSections;

    for (WORD i = 0; i < num_sections; ++i) {
        DWORD va_start = sec[i].VirtualAddress;
        DWORD raw_size = sec[i].SizeOfRawData;
        DWORD virt_size = sec[i].Misc.VirtualSize;
        DWORD span = (virt_size > raw_size) ? virt_size : raw_size;

        if (rva >= va_start && rva < va_start + span) {
            DWORD delta = rva - va_start;
            if (delta >= raw_size) {
                return NULL;
            }
            size_t file_offset = (size_t)sec[i].PointerToRawData + (size_t)delta;
            if (file_offset >= file_size) {
                return NULL;
            }
            return base + file_offset;
        }
    }
    return NULL;
}

static int inspect_pe_buffer(const uint8_t *base, size_t file_size, const char *path) {
    if (file_size < sizeof(IMAGE_DOS_HEADER)) {
        fprintf(stderr, "[!] File too small for IMAGE_DOS_HEADER: %s\n", path);
        return 1;
    }

    const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        fprintf(stderr, "[!] Invalid DOS MZ signature (0x%04X)\n", dos->e_magic);
        return 2;
    }

    if (dos->e_lfanew < 0 || (size_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > file_size) {
        fprintf(stderr, "[!] Invalid e_lfanew offset\n");
        return 3;
    }

    const IMAGE_NT_HEADERS64 *nt = (const IMAGE_NT_HEADERS64 *)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        fprintf(stderr, "[!] Invalid PE signature (0x%08lX)\n", (unsigned long)nt->Signature);
        return 4;
    }

    if (nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        fprintf(stderr, "[!] Only PE32+ (64-bit) images are supported (Magic=0x%04X)\n",
                nt->OptionalHeader.Magic);
        return 5;
    }

    WORD dll_chars = nt->OptionalHeader.DllCharacteristics;
    printf("================================================================\n");
    printf("PE32+ Structural Report : %s\n", path);
    printf("================================================================\n");
    printf("Machine                 : 0x%04X (AMD64)\n", nt->FileHeader.Machine);
    printf("NumberOfSections        : %u\n", nt->FileHeader.NumberOfSections);
    printf("ImageBase               : 0x%016llX\n",
           (unsigned long long)nt->OptionalHeader.ImageBase);
    printf("AddressOfEntryPoint     : 0x%08lX\n",
           (unsigned long)nt->OptionalHeader.AddressOfEntryPoint);
    printf("DllCharacteristics      : 0x%04X [HIGH_ENTROPY_VA=%s, DYNAMIC_BASE=%s, NX_COMPAT=%s]\n",
           dll_chars,
           (dll_chars & IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA) ? "YES" : "NO",
           (dll_chars & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) ? "YES" : "NO",
           (dll_chars & IMAGE_DLLCHARACTERISTICS_NX_COMPAT) ? "YES" : "NO");

    /* Dump Sections */
    printf("\n[Sections]\n");
    const IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        char name[9] = {0};
        for (int j = 0; j < 8 && sec[i].Name[j] != '\0'; ++j) {
            name[j] = (char)sec[i].Name[j];
        }
        printf("  %-8s VA=0x%08lX VSize=0x%08lX RawPtr=0x%08lX RawSize=0x%08lX Chars=0x%08lX\n",
               name,
               (unsigned long)sec[i].VirtualAddress,
               (unsigned long)sec[i].Misc.VirtualSize,
               (unsigned long)sec[i].PointerToRawData,
               (unsigned long)sec[i].SizeOfRawData,
               (unsigned long)sec[i].Characteristics);
    }

    /* Dump Export Directory */
    const IMAGE_DATA_DIRECTORY *exp_dir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    printf("\n[Export Directory] RVA=0x%08lX Size=0x%08lX\n",
           (unsigned long)exp_dir->VirtualAddress, (unsigned long)exp_dir->Size);

    if (exp_dir->VirtualAddress != 0 && exp_dir->Size >= sizeof(IMAGE_EXPORT_DIRECTORY)) {
        const IMAGE_EXPORT_DIRECTORY *exports = (const IMAGE_EXPORT_DIRECTORY *)
            rva_to_ptr(base, file_size, nt, exp_dir->VirtualAddress);
        if (exports != NULL) {
            const char *mod_name = (const char *)rva_to_ptr(base, file_size, nt, exports->Name);
            printf("  Module Name     : %s\n", mod_name ? mod_name : "<unknown>");
            printf("  NumberOfNames   : %lu\n", (unsigned long)exports->NumberOfNames);

            const DWORD *name_rvas =
                (const DWORD *)rva_to_ptr(base, file_size, nt, exports->AddressOfNames);
            const WORD *ordinals =
                (const WORD *)rva_to_ptr(base, file_size, nt, exports->AddressOfNameOrdinals);
            const DWORD *functions =
                (const DWORD *)rva_to_ptr(base, file_size, nt, exports->AddressOfFunctions);

            if (name_rvas && ordinals && functions) {
                for (DWORD i = 0; i < exports->NumberOfNames; ++i) {
                    const char *fn_name =
                        (const char *)rva_to_ptr(base, file_size, nt, name_rvas[i]);
                    WORD ord_idx = ordinals[i];
                    DWORD fn_rva = (ord_idx < exports->NumberOfFunctions) ? functions[ord_idx] : 0;
                    printf("    [%02lu] Ord=%u RVA=0x%08lX Name=%s\n",
                           (unsigned long)i,
                           (unsigned)(exports->Base + ord_idx),
                           (unsigned long)fn_rva,
                           fn_name ? fn_name : "<invalid>");
                }
            }
        }
    }

    /* Dump Import Directory */
    const IMAGE_DATA_DIRECTORY *imp_dir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    printf("\n[Import Directory] RVA=0x%08lX Size=0x%08lX\n",
           (unsigned long)imp_dir->VirtualAddress, (unsigned long)imp_dir->Size);

    if (imp_dir->VirtualAddress != 0 && imp_dir->Size >= sizeof(IMAGE_IMPORT_DESCRIPTOR)) {
        const IMAGE_IMPORT_DESCRIPTOR *imp = (const IMAGE_IMPORT_DESCRIPTOR *)
            rva_to_ptr(base, file_size, nt, imp_dir->VirtualAddress);
        while (imp != NULL && imp->Name != 0) {
            const char *dll_name = (const char *)rva_to_ptr(base, file_size, nt, imp->Name);
            printf("  DLL: %s\n", dll_name ? dll_name : "<invalid>");
            ++imp;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    const char *target = (argc > 1) ? argv[1] : "benign.dll";

    FILE *fp = fopen(target, "rb");
    if (fp == NULL) {
        fprintf(stderr, "[!] Failed to open file: %s\n", target);
        return 1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 1;
    }
    long len = ftell(fp);
    if (len <= 0) {
        fclose(fp);
        return 1;
    }
    rewind(fp);

    uint8_t *buffer = (uint8_t *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (size_t)len);
    if (buffer == NULL) {
        fclose(fp);
        return 1;
    }

    size_t read_bytes = fread(buffer, 1, (size_t)len, fp);
    fclose(fp);

    int rc = 1;
    if (read_bytes == (size_t)len) {
        rc = inspect_pe_buffer(buffer, read_bytes, target);
    }

    HeapFree(GetProcessHeap(), 0, buffer);
    return rc;
}
