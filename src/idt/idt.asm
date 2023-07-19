section .asm

global int21h
global idt_load
global no_interrupt
global enable_interupts
global disable_interupts
global isr80h_wrapper

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

disable_interupts:
    cli
    ret

enable_interupts:
    sti
    ret

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

isr80h_wrapper:
    cli
    pushad

    push esp
    ; eax will contain the kernel command to invoke
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax ; return result
    add esp, 8
   
    popad
    sti
    iretd

section .data
; used to store the return result of isr80h_handler
tmp_res: dd 0