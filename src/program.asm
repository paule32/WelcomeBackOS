BITS 32
section .text
global _user_program
_user_program:

   mov [0x000b8000], byte 'T'
   mov [0x000b8002], byte 'e'
   mov [0x000b8004], byte 's'
   mov [0x000b8006], byte 't'
   mov [0x000b8008], byte ' '
   
   mov [0x000b8000 + 10], byte 'T'
   mov [0x000b8000 + 12], byte 'e'
   mov [0x000b8000 + 14], byte 's'
   mov [0x000b8000 + 16], byte 't'
   mov [0x000b8000 + 18], byte ' '
   
   mov [0x000b8000 + 20], byte 'T'
   mov [0x000b8000 + 22], byte 'e'
   mov [0x000b8000 + 24], byte 's'
   mov [0x000b8000 + 26], byte 't'
   mov [0x000b8000 + 28], byte ' '
   
   mov [0x000b8000 + 30], byte 'T'
   mov [0x000b8000 + 32], byte 'e'
   mov [0x000b8000 + 34], byte 's'
   mov [0x000b8000 + 36], byte 't'
   mov [0x000b8000 + 38], byte ' '

   jmp _user_program
