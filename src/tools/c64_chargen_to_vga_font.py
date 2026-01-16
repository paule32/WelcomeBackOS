#!/usr/bin/env python3
"""
c64_chargen_to_vga_font.py

Converts a Commodore 64 Character ROM dump (chargen, 4096 bytes) into a raw VGA 8x8 user font
(256 glyphs * 8 bytes = 2048 bytes) suitable for BIOS INT 10h AX=1110h (BH=8, CX=256).

You must provide your OWN legally obtained C64 character ROM dump.

Usage:
  python c64_chargen_to_vga_font.py chargen.bin --set 0 --out petscii_set1_8x8.fnt
  python c64_chargen_to_vga_font.py chargen.bin --set 1 --out petscii_set2_8x8.fnt

--set:
  0 = Charset/PETSCII set 1 (offset 0x0000, 2048 bytes)
  1 = Charset/PETSCII set 2 (offset 0x0800, 2048 bytes)
"""

from __future__ import annotations
from pathlib import Path
import argparse
import sys


def die(msg: str, code: int = 1) -> None:
    print(f"ERROR: {msg}", file=sys.stderr)
    raise SystemExit(code)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path, help="C64 character ROM dump (chargen) – must be exactly 4096 bytes")
    ap.add_argument("--set", type=int, choices=[0, 1], default=0, help="0=set 1 (offset 0x0000), 1=set 2 (offset 0x0800)")
    ap.add_argument("--out", type=Path, default=Path("petscii_8x8.fnt"), help="output file (2048 bytes)")
    ap.add_argument("--nasm-inc", type=Path, default=None, help="optional: write a NASM include with db lines (8 bytes per char)")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    if len(data) != 4096:
        die(f"Expected 4096 bytes ROM, got {len(data)} bytes")

    off = 0x0800 * args.set
    font = data[off:off + 2048]
    if len(font) != 2048:
        die("Internal slice error (font length != 2048)")

    args.out.write_bytes(font)
    print(f"Wrote {args.out} ({len(font)} bytes) from ROM offset 0x{off:04X}")

    if args.nasm_inc:
        lines = []
        lines.append("; Auto-generated from C64 chargen ROM")
        lines.append("; 256 glyphs * 8 bytes, byte = 8 pixels (bit7 left .. bit0 right)")
        for ch in range(256):
            b = font[ch*8:(ch+1)*8]
            lines.append(f"; char {ch:3d} (0x{ch:02X})")
            lines.append("db " + ",".join(f"0x{x:02X}" for x in b))
        args.nasm_inc.write_text("\n".join(lines) + "\n", encoding="utf-8")
        print(f"Wrote {args.nasm_inc} (NASM include)")


if __name__ == "__main__":
    main()
