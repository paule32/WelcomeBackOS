#!/usr/bin/env python3
"""
Erzeugt / initialisiert ein 1,44MB-FAT12-Floppy-Image, schreibt einen
Bootsektor hinein und kopiert Dateien aus einem Verzeichnis ins Root.
"""

import argparse
import os
from pathlib import Path

# Geometrie für 1,44MB-FAT12
BYTES_PER_SECTOR = 512
SECTORS_PER_CLUSTER = 1
RESERVED_SECTORS = 1
NUM_FATS = 2
ROOT_DIR_ENTRIES = 224
TOTAL_SECTORS = 2880
MEDIA_DESCRIPTOR = 0xF0
SECTORS_PER_FAT = 9
SECTORS_PER_TRACK = 18
NUM_HEADS = 2

ROOT_DIR_SECTORS = (ROOT_DIR_ENTRIES * 32 + (BYTES_PER_SECTOR - 1)) // BYTES_PER_SECTOR
FIRST_DATA_SECTOR = RESERVED_SECTORS + NUM_FATS * SECTORS_PER_FAT + ROOT_DIR_SECTORS
TOTAL_CLUSTERS = (TOTAL_SECTORS - FIRST_DATA_SECTOR) // SECTORS_PER_CLUSTER
IMAGE_SIZE = TOTAL_SECTORS * BYTES_PER_SECTOR


def create_empty_image(path: Path) -> None:
    if path.exists():
        size = path.stat().st_size
        if size != IMAGE_SIZE:
            raise RuntimeError(
                f"Image already exists, but the size is {size} "
                f"instead {IMAGE_SIZE} Bytes"
            )
        return

    with path.open("wb") as f:
        f.truncate(IMAGE_SIZE)


def write_bootsector(img_path: Path, boot_path: Path) -> None:
    boot = boot_path.read_bytes()
    if len(boot) != BYTES_PER_SECTOR:
        raise RuntimeError(
            f"Bootsektor muss genau {BYTES_PER_SECTOR} "
            f"Bytes haben (ist {len(boot)})"
        )

    with img_path.open("r+b") as img:
        img.seek(0)
        img.write(boot)


def init_fat_and_root(img_path: Path) -> None:
    """Initialisiert FAT-Bereiche, Root-Directory und Data-Area (alles auf 0)."""
    with img_path.open("r+b") as img:
        # FAT-Bereiche nullen
        fat_start = RESERVED_SECTORS * BYTES_PER_SECTOR
        fat_size_bytes = SECTORS_PER_FAT * BYTES_PER_SECTOR
        img.seek(fat_start)
        img.write(b"\x00" * (fat_size_bytes * NUM_FATS))

        # Root-Verzeichnis nullen
        root_start = (RESERVED_SECTORS + NUM_FATS * SECTORS_PER_FAT) * BYTES_PER_SECTOR
        root_size = ROOT_DIR_ENTRIES * 32
        img.seek(root_start)
        img.write(b"\x00" * root_size)

        # Data Area nullen (optional, aber sauber)
        data_start = FIRST_DATA_SECTOR * BYTES_PER_SECTOR
        data_size = (TOTAL_SECTORS - FIRST_DATA_SECTOR) * BYTES_PER_SECTOR
        img.seek(data_start)
        img.write(b"\x00" * data_size)

        # FAT12-Reservereinträge setzen (FAT1, später auf FAT2 gespiegelt)
        fat = bytearray(fat_size_bytes)
        # kompletten FAT1-Block einlesen (gerade genullt)
        img.seek(fat_start)
        img.readinto(fat)

        # Erste drei Bytes für FAT12:
        # Eintrag 0 und 1 (je 12 Bits)
        # Häufig: F0 FF FF
        fat[0] = MEDIA_DESCRIPTOR
        fat[1] = 0xFF
        fat[2] = 0xFF

        # FAT1 zurückschreiben
        img.seek(fat_start)
        img.write(fat)

        # FAT1 auf FAT2 spiegeln
        fat2_start = fat_start + fat_size_bytes
        img.seek(fat2_start)
        img.write(fat)


def load_fat(img_path: Path) -> bytearray:
    fat_start = RESERVED_SECTORS * BYTES_PER_SECTOR
    fat_size_bytes = SECTORS_PER_FAT * BYTES_PER_SECTOR
    with img_path.open("rb") as img:
        img.seek(fat_start)
        fat = bytearray(img.read(fat_size_bytes))
    return fat


def save_fat(img_path: Path, fat: bytearray) -> None:
    fat_start = RESERVED_SECTORS * BYTES_PER_SECTOR
    fat_size_bytes = SECTORS_PER_FAT * BYTES_PER_SECTOR
    with img_path.open("r+b") as img:
        # FAT1
        img.seek(fat_start)
        img.write(fat)
        # FAT2
        fat2_start = fat_start + fat_size_bytes
        img.seek(fat2_start)
        img.write(fat)


def set_fat12_entry(fat: bytearray, cluster: int, value: int) -> None:
    """Setzt einen FAT12-Eintrag (12-Bit-Wert)."""
    if cluster < 0:
        raise ValueError("Cluster must be >= 0")
    if value > 0xFFF:
        raise ValueError("FAT12-Value max is 0xFFF.")

    # FAT12: 3 Bytes enthalten 2 Einträge
    # Byte-Offset:
    offset = (cluster * 3) // 2

    if cluster & 1 == 0:
        # gerader Cluster: low 12 Bits
        fat[offset] = value & 0xFF
        fat[offset + 1] = (fat[offset + 1] & 0xF0) | ((value >> 8) & 0x0F)
    else:
        # ungerader Cluster: high 12 Bits
        fat[offset] = (fat[offset] & 0x0F) | ((value << 4) & 0xF0)
        fat[offset + 1] = (value >> 4) & 0xFF


def get_fat12_entry(fat: bytearray, cluster: int) -> int:
    offset = (cluster * 3) // 2
    if cluster & 1 == 0:
        # even
        value = fat[offset] | ((fat[offset + 1] & 0x0F) << 8)
    else:
        value = ((fat[offset] & 0xF0) >> 4) | (fat[offset + 1] << 4)
    return value


def find_free_clusters(fat: bytearray, needed: int) -> list[int]:
    """Sucht 'needed' freie Cluster (Wert 0) und gibt deren Nummern zurück."""
    result: list[int] = []
    cluster = 2  # FAT12: Cluster 0 und 1 reserviert

    while cluster < TOTAL_CLUSTERS + 2 and len(result) < needed:
        if get_fat12_entry(fat, cluster) == 0:
            result.append(cluster)
        cluster += 1

    if len(result) < needed:
        raise RuntimeError("Nicht genug freie Cluster im Image")

    return result


def write_file_data(img_path: Path, file_path: Path, clusters: list[int]) -> None:
    data = file_path.read_bytes()
    bytes_per_cluster = BYTES_PER_SECTOR * SECTORS_PER_CLUSTER

    with img_path.open("r+b") as img:
        for i, cl in enumerate(clusters):
            chunk = data[i * bytes_per_cluster:(i + 1) * bytes_per_cluster]
            if len(chunk) < bytes_per_cluster:
                chunk = chunk + b"\x00" * (bytes_per_cluster - len(chunk))
            offset = (FIRST_DATA_SECTOR + (cl - 2) * SECTORS_PER_CLUSTER) * BYTES_PER_SECTOR
            img.seek(offset)
            img.write(chunk)


def format_8_3(name: str) -> tuple[bytes, bytes]:
    """Wandelt Dateinamen in 8.3-Format (Großbuchstaben, gepadded) um."""
    name = name.upper()
    if "." in name:
        base, ext = name.split(".", 1)
    else:
        base, ext = name, ""

    # unerlaubte Zeichen ersetzen
    def clean(s: str) -> str:
        allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!#$%&'()-@^_`{}~"
        return "".join(c if c in allowed else "_" for c in s)

    base = clean(base)[:8]
    ext = clean(ext)[:3]

    base_bytes = base.ljust(8).encode("ascii", errors="replace")
    ext_bytes = ext.ljust(3).encode("ascii", errors="replace")
    return base_bytes, ext_bytes


def write_root_directory_entry(
    img_path: Path,
    index: int,
    file_path: Path,
    first_cluster: int) -> None:
    """Schreibt einen Eintrag ins Root-Directory (Index 0..ROOT_DIR_ENTRIES-1)."""
    if index >= ROOT_DIR_ENTRIES:
        raise RuntimeError("too many files for root directory")

    root_start = (RESERVED_SECTORS + NUM_FATS * SECTORS_PER_FAT) * BYTES_PER_SECTOR
    entry_offset = root_start + index * 32

    name_bytes, ext_bytes = format_8_3(file_path.name)
    size = file_path.stat().st_size

    entry = bytearray(32)
    entry[0:8] = name_bytes
    entry[8:11] = ext_bytes
    entry[11] = 0x20  # Attribut: Archive

    # Zeiten/Datumswerte lassen wir 0 -> DOS interpretiert das als "kein Datum"
    # Erstes Cluster
    entry[26] = first_cluster & 0xFF
    entry[27] = (first_cluster >> 8) & 0xFF

    # Dateigröße
    entry[28] = size & 0xFF
    entry[29] = (size >> 8) & 0xFF
    entry[30] = (size >> 16) & 0xFF
    entry[31] = (size >> 24) & 0xFF

    with img_path.open("r+b") as img:
        img.seek(entry_offset)
        img.write(entry)


def populate_image_with_directory(
    img_path: Path,
    src_dir: Path) -> None:
    """Kopiert alle Dateien aus src_dir ins Root-FAT12-Verzeichnis."""
    if not src_dir.is_dir():
        raise RuntimeError(f"{src_dir} is not a directory")

    files = [p for p in src_dir.iterdir() if p.is_file()]
    files.sort()

    fat = load_fat(img_path)

    root_index = 0
    for f in files:
        size = f.stat().st_size
        if size == 0:
            # Leere Dateien: kein Cluster, aber Root-Entry mit Cluster 0 ist üblich
            first_cluster = 0
            write_root_directory_entry(img_path, root_index, f, first_cluster)
            root_index += 1
            continue

        bytes_per_cluster = BYTES_PER_SECTOR * SECTORS_PER_CLUSTER
        needed_clusters = (size + bytes_per_cluster - 1) // bytes_per_cluster

        clusters = find_free_clusters(fat, needed_clusters)

        # FAT-Kette setzen
        for i, cl in enumerate(clusters):
            if i == len(clusters) - 1:
                val = 0xFFF  # End-of-chain
            else:
                val = clusters[i + 1]
            set_fat12_entry(fat, cl, val)

        # Dateidaten schreiben
        write_file_data(img_path, f, clusters)

        # Root-Dir-Eintrag
        write_root_directory_entry(img_path, root_index, f, clusters[0])
        root_index += 1

    # FAT zurückschreiben (auf beide Kopien)
    save_fat(img_path, fat)


def main() -> None:
    parser = argparse.ArgumentParser(description="create/fill a FAT12-Floppy-Image.")
    parser.add_argument("--image", "-i", type=Path, required=True, help="path to the floppy image.")
    parser.add_argument("--boot" , "-b", type=Path, required=True, help="bootsector-file (512 Bytes).")
    parser.add_argument("--src"  , "-s", type=Path, required=True, help="source directory (Root of FAT12).")

    args = parser.parse_args()

    create_empty_image           (args.image           )
    write_bootsector             (args.image, args.boot)
    init_fat_and_root            (args.image           )
    populate_image_with_directory(args.image, args.src )

    print(f"Donw: {args.image} with boot sector: {args.boot} and files from: {args.src}")


if __name__ == "__main__":
    main()
