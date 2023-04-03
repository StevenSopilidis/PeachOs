section .asm

global int21h
global idt_load
global no_interrupt

extern int21h_handler
extern no_interrupt_handler

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; first argument that we passed
    lidt [ebx]

    pop ebp 
    ret 

int21h: ; 21fist interupt
    cli
    pushad

    call int21h_handler ; handler from c

    popad
    sti
    iret ; return from interupt

no_interrupt: ; when no interupt is set 
    cli
    pushad

    call no_interrupt_handler ; handler from c

    popad
    sti
    iret ; return from interupt