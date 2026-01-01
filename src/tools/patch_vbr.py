import sys

# Usage:
#   python tools/patch_vbr.py <image> <part_start_sector> <vbr_bin> 
# Patches BPB (first 90 bytes) from existing FAT32 boot sector into our VBR code.

def main():
    if len(sys.argv) != 4:
        print("usage: patch_vbr.py <img> <part_start_sector> <vbr_bin>")
        return 2

    img_path = sys.argv[1]
    part_start = int(sys.argv[2])
    vbr_path = sys.argv[3]

    with open(img_path, "rb") as f:
        f.seek(part_start * 512)
        orig = bytearray(f.read(512))
    if len(orig) != 512 or orig[510:512] != b"\x55\xAA":
        raise SystemExit("partition boot sector not valid")

    with open(vbr_path, "rb") as f:
        vbr = bytearray(f.read(512))
    if len(vbr) != 512 or vbr[510:512] != b"\x55\xAA":
        raise SystemExit("vbr bin must be exactly 512 bytes and end with 55AA")

    # Copy BPB+EBPB area (we reserved 90 bytes)
    vbr[0:90] = orig[0:90]

    with open(vbr_path, "wb") as f:
        f.write(vbr)

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
