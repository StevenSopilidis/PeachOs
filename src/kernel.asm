[BITS 32]
global _start
; code and data segments we have setup
CODE_SEG equ 0x08
DATA_SEG equ 0x08

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; use FAST A20 to enable A20 line
    in al, 0x92
    or al, 2
    out 0x92, al


    jmp $