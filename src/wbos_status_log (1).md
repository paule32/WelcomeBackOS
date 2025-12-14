# Development Status Log – WelcomeBackOS

## 1️⃣ Bootprozess: Stage 1 & Stage 2 erfolgreich implementiert
### Stage 1
- Real-Mode Bootsektor
- INT 13h Extensions (EDD) Tests
- LBA-Read Tests
- Laden von Stage2
- BIOS Debug-Ausgabe

### Stage 2
- Läuft bei 0x0000:0500
- Setzen der Segmente
- Laden des Kernels via LBA
- A20 aktiv
- GDT aufgebaut
- Protected Mode aktiviert
- Kernelstart bei 0x00010000

## 2️⃣ Kernel
- Makefile für 32-Bit Cross-Compilation
- kernel.ld erzeugt Flat Binary
- VGA-Ausgabe funktioniert
- PM32 stabil

## 3️⃣ Paging
- Identity Mapping der unteren 4 MiB
- Page Directory + Page Tables
- CR3/CR0 korrekt gesetzt

## 4️⃣ Heap
- Grundstruktur implementiert
- Alignment-Fixes
- Vorbereitung auf malloc/free

## 5️⃣ GDT & TSS
- GDT vollständig
- TSS korrekt initialisiert
- ltr erfolgreich

## 6️⃣ Interrupts
- IDT voll aufgebaut
- Exceptions, IRQs
- PIC remapped
- Syscalls vorbereitet

## 7️⃣ Usermode
- enter_usermode funktioniert bis ASM
- Direkter Funktionsaufruf klappt
- Ring 3 via iret in Arbeit

## 8️⃣ ISO Boot
- Boot ohne boot-info-table
- ISO bootet korrekt
- LBA geprüft
