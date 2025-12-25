; ---------------------------------------------------------------------------
; \file  data.asm
; \note  (c) 2025 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
; data for ramdisk
bits 32
section .text

global _file_data_start
global _file_data_end
_file_data_start:
incbin "initrd.dat"
_file_data_end:

