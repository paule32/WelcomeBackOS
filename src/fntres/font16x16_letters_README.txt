font16x16_letters.h
===================

Enthält 16x16 1-bit Bitmasken für alle Buchstaben A-Z und a-z.

Format:
- uint16_t font16x16_letters[52][16]
- Jede Glyphe: 16 Zeilen, jeweils 16 Bits.
- Bit 15 ist das linke Pixel (x=0), Bit 0 das rechte Pixel (x=15).

Index:
- 'A'..'Z' -> 0..25
- 'a'..'z' -> 26..51

Beispiel (RGB565):
------------------
for (int row=0; row<16; row++) {
    uint16_t bits = font16x16_letters[idx][row];
    for (int col=0; col<16; col++) {
        if (bits & (1u << (15-col))) put565(dst, x+col, y+row, color);
    }
}
