section .asm

global idt_load

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; first argument that we passed
    lidt [ebx]

    pop ebp 
    ret 