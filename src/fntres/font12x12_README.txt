12x12 Bitmasken-Fonts (1-bit)
============================

Enthalten:
- font12x12_letters.h : A-Z und a-z (52 Glyphen)
- font12x12_extra.h   : Ziffern, ASCII-Sonderzeichen, deutsche Umlaute (ÄÖÜäöüß) + ein paar Extras

Format:
- Jede Glyphe hat 12 Zeilen.
- Jede Zeile ist ein uint16_t.
- Es werden nur die Bits 15..4 benutzt:
    Bit 15 = x=0 (links), Bit 4 = x=11 (rechts)
  (die unteren 4 Bits sind immer 0)

Zeichnen:
for (int row=0; row<12; row++) {
    uint16_t bits = glyph[row];
    for (int col=0; col<12; col++) {
        if (bits & (1u << (15-col))) put565(dst, x+col, y+row, color);
    }
}

Lookup:
- Letters: font12x12_letter_index('A'..'Z'/'a'..'z')
- Extra  : font12x12_extra_index(codepoint), via font12x12_extra_codepoints[]

UTF-8 Hinweis:
- Umlaute in UTF-8 sind mehrbyte Zeichen, du musst UTF-8 -> Unicode Codepoints dekodieren.
