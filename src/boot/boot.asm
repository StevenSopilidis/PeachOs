ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start ; offset of code segment in gdt
DATA_SEG equ gdt_data - gdt_start ; offset of data segment in gdt

; bios parameter block set
_start: 
    jmp short start
    nop
times 33 db 0

; start of bootloader$
start: 
    jmp 0:step2 ; cs == 0 * 16 + offset of step2 (origin is set to 0x7c00)

step2:
    cli
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00 ; stack location == ss * 16 + sp
    sti

.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1 ; enable bit of cr0 to go to protected mode
    mov cr0, eax
    jmp CODE_SEG:load32

;GDT
gdt_start:
gdt_null: ; first entry of GDT which must be null
    dd 0x0
    dd 0x0
gdt_code: ; offset 0x8 (CS should point to this)
    dw 0xffff ; segment limit first 0-15 bits
    dw 0 ; Base first 0-15 bits
    db 0 ; Base 16-23 bits
    db 0x9a ; Access Byte
    db 11001111b ; High 4 bits of flag and low 4 bit flags
    db 0 ; Base 24-31 bits
gdt_data: ; offset 0x10 (DS,SS,ES,FS,GS point to this)
    dw 0xffff ; segment limit first 0-15 bits
    dw 0 ; Base first 0-15 bits
    db 0 ; Base 16-23 bits
    db 0x92 ; Access Byte
    db 11001111b ; High 4 bits of flag and low 4 bit flags
    db 0 ; Base 24-31 bits
gdt_end:
gdt_descriptor: 
    dw gdt_end - gdt_start - 1 ;size of gdt 
    dd gdt_start ; offset of gdt

[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
    jmp $

times 510 - ($ - $$) db 0
dw 0xaa55 ; 0x55aa -> x86_64 architecture is little endian so we right it in reverse

buffer: 