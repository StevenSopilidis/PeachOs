[BITS 32]

section .asm
global _start

_start:
    push message
    mov eax, 1 ; Print command
    int 0x80
    add esp, 4 ; restore stack

    jmp $

section .data
message: db "Hallo! from the users process", 0