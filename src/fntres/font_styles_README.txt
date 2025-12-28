Fonts generated:

font12x12_regular_letters.h
font12x12_regular_extra.h
font12x12_bold_letters.h
font12x12_bold_extra.h
font12x12_italic_letters.h
font12x12_italic_extra.h
font16x16_regular_letters.h
font16x16_regular_extra.h
font16x16_bold_letters.h
font16x16_bold_extra.h
font16x16_italic_letters.h
font16x16_italic_extra.h

Format:
- Each glyph is H rows of uint16_t.
- MSB (bit15) = x=0 (left).
- For 12x12 only bits 15..4 are used (aligned left).

Lookup:
- Letters: <name>_letter_index(char c) for ASCII A-Z/a-z
- Extra  : <name>_index(uint32_t codepoint) using *_codepoints table

UTF-8: decode to Unicode codepoints before calling *_index() for umlauts.