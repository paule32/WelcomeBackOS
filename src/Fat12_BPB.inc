%ifndef FAT12_BPB_INC
%define FAT12_BPB_INC

bits 16

; "FAT 12" file system is build up by four areas at Floppy Disk:
; - Boot Sector (also called BIOS Parameter Block, BPB) 
; - File Allocation Table, FAT 
; - Root Directory
; - Subdirectories and Files

; boot sector info block

; --- BIOS Parameter Block (BPB) ---
OperatingSystemName: db "MSDOS5.0"          ; OEM

BytesPerSec:         dw 512
SecPerClus:          db 4
ReservedSec:         dw 1
NumFATs:             db 2
RootEntries:         dw 512
TotSec:              dw 0
MediaType:           db 0xF8
FATSize:             dw 128
SecPerTrack:         dw 32
NumHeads:            dw 64
HiddenSec:           dd 0
TotSec32:            dd 32768

; --- Extended BPB (DOS 4.0+) ---
DriveNum:            db 0x80
Reserved:            db 0
BootSig:             db 0x29
VolumeSerialNum:     dd 0x12345678
VolumeLabel:         db "NO NAME    "
FileSys:             db "FAT16   "     ;  8 byte

%endif
