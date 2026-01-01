import struct
import sys

# usage:
#   python tools/make_mbr_partition.py <img> <part_start_sector> <total_size_mib>

def main():
    if len(sys.argv) != 4:
        print("usage: make_mbr_partition.py <img> <part_start_sector> <total_size_mib>")
        return 2

    img = sys.argv[1]
    start = int(sys.argv[2])
    size_mib = int(sys.argv[3])

    size_bytes = size_mib * 1024 * 1024
    total_sectors = size_bytes // 512
    part_sectors = total_sectors - start
    if part_sectors <= 0:
        raise SystemExit("partition too small")

    mbr = bytearray(512)

    # One active FAT32 LBA partition (type 0x0C)
    # status, chs_first(3), type, chs_last(3), lba_first, sectors
    pe = struct.pack("<B3sB3sII",
                     0x80, b"\0\2\0", 0x0C, b"\0\0\0",
                     start, part_sectors)
    mbr[446:446+16] = pe
    mbr[510:512] = b"\x55\xAA"

    with open(img, "r+b") as f:
        f.seek(0)
        f.write(mbr)

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
