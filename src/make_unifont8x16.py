#!/usr/bin/env python3
import sys

def parse_unifont_hex(hex_path, out_path):
    # 65536 Codepoints (BMP)
    font = [[0] * 16 for _ in range(0x10000)]

    with open(hex_path, "r", encoding="ascii") as f:
        for line in f:
            line = line.strip()
            if not line or ":" not in line:
                continue

            code_str, bitmap_hex = line.split(":")
            code = int(code_str, 16)
            if code < 0 or code > 0xFFFF:
                continue

            # 8x16 => 16 Bytes = 32 Hex-Zeichen
            # 16x16 => 32 Bytes = 64 Hex-Zeichen (die lassen wir hier einfach aus)
            if len(bitmap_hex) == 32:
                # 8 Pixel breit, 16 Pixel hoch
                rows = [int(bitmap_hex[i:i+2], 16) for i in range(0, 32, 2)]
                if len(rows) == 16:
                    font[code] = rows
            else:
                # 16x16 oder irgendwas anderes -> ignorieren in diesem 8x16-Font
                continue

    with open(out_path, "w", encoding="ascii") as out:
        out.write("// Auto-generiert aus GNU Unifont (.hex)\n")
        out.write("// Jede Glyphe: 8x16, 1 Bit pro Pixel\n\n")
        out.write("#pragma once\n\n")
        out.write("#include \"os.h\"\n\n")
        out.write("static const UCHAR font8x16[65536][16] = {\n")

        for code in range(0x10000):
            rows = font[code]
            row_str = ", ".join(f"0x{b:02X}" for b in rows)
            out.write(f"  /* U+{code:04X} */ {{ {row_str} }},\n")

        out.write("};\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: make_unifont8x16.py unifont.hex font8x16.h")
        sys.exit(1)

    parse_unifont_hex(sys.argv[1], sys.argv[2])
