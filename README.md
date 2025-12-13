# WelcomeBackOS – Bootloader & 32-Bit Kernel Build from Scratch
(c) 2025 Jens Kallup – paule32  
Alle Rechte vorbehalten.

Dieses Projekt zeigt Schritt für Schritt, wie man:

- einen **El-Torito CD-Bootloader** (Stage1 & Stage2) schreibt  
- einen **32-Bit Protected-Mode Kernel** lädt  
- A20 aktiviert, GDT setzt und PM startet  
- die Tools **NASM**, **MinGW-w64**, **LD**, **OBJCOPY**, **xorriso** installiert und nutzt  
- ein komplettes Build-System aus **Makefile** und **ISO-Erstellungs-Script** aufsetzt  

Damit können Entwickler ein vollständiges eigenes Betriebssystem booten – auf echten PCs, QEMU, VirtualBox usw.

---

# 📦 1. Voraussetzungen / Downloads / Installation

## 1.1 NASM herunterladen und installieren
NASM ist der Assembler für Stage1 & Stage2.

Download (Win64):  
https://www.nasm.us/pub/nasm/releasebuilds/

Installation:
1. ZIP herunterladen  
2. nach `C:\Tools\nasm\` entpacken  
3. PATH erweitern:

```
set PATH=C:\Tools\nasm;%PATH%
```

---

## 1.2 MinGW-w64 installieren (für gcc, ld, objcopy)

Download:  
https://www.mingw-w64.org/downloads/

Empfohlen: Variante “posix-seh”.

---

## 1.3 xorriso installieren

Unter MSYS2:

```
pacman -S xorriso
```

---

# 🚀 2. Bootprozess-Überblick

```
BIOS → Stage1 → Stage2 → Protected Mode → 32-Bit Kernel
```

Stage1: lädt Stage2  
Stage2: lädt Kernel, aktiviert A20, wechselt in Protected Mode  
Kernel: läuft im 32-Bit Modus

---

# 🔢 3. LBA & Sektor-Bestimmung

Nach ISO-Erstellung:

```
xorriso -indev bootcd.iso -find /kernel.bin -exec report_lba --
```

Beispiel:

```
File data lba: 0 , 38 , 23 , ... , '/kernel.bin'
```

→ LBA = 38  
→ SECTORS = 23

---

# 🧠 4. Speicherlayout & ORG

| Komponente | BIOS-Adresse | ORG | Kommentar |
|-----------|--------------|--------|-----------|
| Stage1 | 0000:0000 | 0x0000 | El Torito lädt hier |
| Stage2 | 0000:0500 | 0x0500 | Stage1 lädt Stage2 hierhin |
| Kernel | phys 0x10000 | 0x0000 | Stage2 lädt Kernel → PM Jump |

---

# 🛠 5. Protected Mode Schritte

Stage2:

1. Kernel per LBA laden  
2. A20 aktivieren  
3. GDT laden (`lgdt`)  
4. CR0.PE setzen  
5. Far-Jump zu 32-Bit Code  
6. Sprung zu Kernel bei physisch `0x00010000`

---

# ⚙ 6. Makefile & Buildsystem

Dieses Projekt verwendet ein umfangreiches Makefile (siehe Repository).

---

# 💽 7. ISO-Erstellung

Script `create_iso.sh`:

```
xorriso -as mkisofs -o bootcd.iso     -b boot1.bin     -no-emul-boot     -boot-load-size 4     .
```

**Wichtig:** Kein `-boot-info-table`, sonst wird boot1.bin beschädigt.

---

# 🎉 8. Ergebnis

Ein vollständiger Bootprozess:

- Stage1 → Stage2 → Protected Mode → Kernel  
- Vollständiger 32-Bit Systemstart  
- Debug-/LBA-Erkennung  
- Reproduzierbar auf jedem System

---

# 📚 9. Lizenz

(c) 2025 Jens Kallup – paule32.
