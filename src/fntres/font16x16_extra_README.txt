font16x16_extra.h
=================

Enthält 16x16 1-bit Bitmasken für:
- Ziffern: 0..9
- ASCII-Sonderzeichen (inkl. Leerzeichen): !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
- Deutsche Umlaute: Ä Ö Ü ä ö ü ß
- Zusätzlich (falls benötigt): € „ “ ‚ ‘ ’ – — … ° §

Format:
- uint16_t font16x16_extra[COUNT][16]
- Bit 15 ist links (x=0), Bit 0 rechts (x=15)

Zusätzlich:
- uint32_t font16x16_extra_codepoints[COUNT]
- Lookup: font16x16_extra_index(codepoint)

Hinweis UTF-8:
- Umlaute sind in UTF-8 mehrbyte-Zeichen (z.B. 'ä' = 0xC3 0xA4).
  Du musst also UTF-8 -> Unicode-Codepoints dekodieren, und dann mappen.

Zeichnen (Pseudo):
for (int row=0; row<16; row++) {
    uint16_t bits = font16x16_extra[idx][row];
    for (int col=0; col<16; col++) {
        if (bits & (1u << (15-col))) put565(dst, x+col, y+row, color);
    }
}
