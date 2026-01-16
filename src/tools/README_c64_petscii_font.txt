C64 PETSCII Bitmasks (8x8) – Generator

Why you don't get the ROM bitmasks directly:
- The original C64 character ROM glyph bytes are typically copyrighted ROM content.
- But you *can* legally convert a ROM dump you own.

What this does:
- Input:  C64 chargen ROM dump (4096 bytes)
- Output: VGA raw 8x8 font (2048 bytes = 256*8)

Create set 1 and set 2:
  python c64_chargen_to_vga_font.py chargen.bin --set 0 --out petscii_set1_8x8.fnt
  python c64_chargen_to_vga_font.py chargen.bin --set 1 --out petscii_set2_8x8.fnt

Optional NASM include:
  python c64_chargen_to_vga_font.py chargen.bin --set 0 --nasm-inc petscii_set1.inc

Load into VGA (BIOS):
- Use INT 10h AX=1110h, BH=8, CX=256, DX=0, ES:BP -> font data
