# include "stdint.h"
# include "kheap.h"
# include "proto.h"

int pe_load_image(const uint8_t* file, uint32_t size, pe_image_t* out)
{
    if (!file || size < sizeof(dos_header_t))
        return -1;

    const dos_header_t* dos = (const dos_header_t*)file;
    if (dos->e_magic != 0x5A4D)   // 'MZ'
        return -2;

    if (dos->e_lfanew <= 0 || (uint32_t)dos->e_lfanew + sizeof(pe_signature_t) > size)
        return -3;

    const uint8_t* pe_ptr = file + dos->e_lfanew;
    const pe_signature_t* sig = (const pe_signature_t*)pe_ptr;
    if (sig->Signature != 0x00004550)   // 'PE\0\0'
        return -4;

    const coff_header_t* coff = (const coff_header_t*)(pe_ptr + sizeof(pe_signature_t));
    if (coff->Machine != 0x14C)        // IMAGE_FILE_MACHINE_I386
        return -5;

    const optional_header_t* opt = (const optional_header_t*)((const uint8_t*)coff + sizeof(coff_header_t));
    if (opt->Magic != 0x10B)           // PE32
        return -6;

    uint16_t nsec = coff->NumberOfSections;
    const section_header_t* sec = (const section_header_t*)((const uint8_t*)opt + coff->SizeOfOptionalHeader);

    uint32_t image_base = opt->ImageBase;
    uint32_t image_size = opt->SizeOfImage;

image_size = 2048;
    // für die erste Version: wir allozieren einfach einen zusammenhängenden Block im Kernel
    // (später → eigenes Page Directory + korrektes Mapping)
    uint8_t* image = (uint8_t*)kmalloc(image_size);
    if (!image)
        return -7;

    kmemset(image, 0, image_size);

    // Header kopieren (optional, nicht zwingend)
    uint32_t hdr_size = opt->SizeOfHeaders;
    if (hdr_size > size)
        hdr_size = size;
    kmemcpy(image, file, hdr_size);

    // Sections kopieren
    for (uint16_t i = 0; i < nsec; ++i) {
        uint32_t vaddr = sec[i].VirtualAddress;
        uint32_t vsize = sec[i].VirtualSize;
        uint32_t raw_size = sec[i].SizeOfRawData;
        uint32_t raw_ptr = sec[i].PointerToRawData;

vaddr = raw_size = 0x200;
        if (raw_size == 0)
            continue;

        if (raw_ptr + raw_size > size)
            return -8;
printformat("RAWS: 0x%x, VADDR: 0x%x\n", raw_size, vaddr);
        if (vaddr + raw_size > image_size)
            return -9;

        kmemcpy(image + vaddr, file + raw_ptr, raw_size);

        // falls VirtualSize > RawSize: Rest bleibt 0 (BSS)
        if (vsize > raw_size) {
            kmemset(image + vaddr + raw_size, 0, vsize - raw_size);
        }
    }

    uint32_t ep = opt->AddressOfEntryPoint;   // RVA
    out->image_base  = (uint32_t)image;       // Achtung: NICHT opt->ImageBase, sondern reale Adresse
    out->image_size  = image_size;
    printformat("PE 1111\n");
    out->entry_point = (uint32_t)(image + ep);
    printformat("PE 2222\n");

    return 0;
}
