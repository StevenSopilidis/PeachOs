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

    jmp $

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

load32: ; load kernel to memory and jump to it
    mov eax, 1 ; starting sector (first sector is zero)
    mov ecx, 100 ; number of sectors to load
    mov edi, 0x0100000 ; location of where the kernel will be loaded
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax ; BACKUP LBA
    ; send the highest 8 bits of the lba to hard disk controller
    shr eax, 24
    or eax, 0xE0 ; Select the master drive
    mov dx, 0x1F6
    out dx, al
    ; finished sending highest 8 bits of the lba

    ; send the total number of sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; end of sending the total number of sectors to read

    mov eax, ebx ; restore backup lba

    ; send more bits of lba
    mov eax, ebx
    mov dx, 0x1F3
    out dx, al
    ; finished sending more bits of the lba

    ; send more bits of lba
    mov dx, 0x1F4
    mov eax, ebx ; Restore the backup lba
    shr eax, 8  
    out dx, al
    ; finished sending more bits of lba

    ; send upper 16 bits of lba
    mov dx, 0x1F5
    mov eax, ebx ; Reswtore backup lba
    shr eax, 16
    out dx, al
    ; finished sending upper 16 bits of lba

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

; Read all sectors into memory
.next_sector:
    push ecx
; check if we are ready to read
.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again
; Read 256 words = 512 bytes at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw ;Input word from I/O port specified in DX into memory location specified in ES:(E)DI (edi is set above to 0x0100000)
    pop ecx
    loop  .next_sector ; loops 256 times ---> read a hole sector

    ; End of reading sectors into memory
    ret


times 510 - ($ - $$) db 0
dw 0xaa55 ; 0x55aa -> x86_64 architecture is little endian so we right it in reverse

buffer: 