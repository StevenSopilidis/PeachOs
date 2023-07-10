[BITS 32]

section .asm

global restore_general_purpose_registers
global task_return
global user_registers


; void task_return(struct registers* regs);
task_return:
    mov ebp, esp
    ; push the data segment (SS will be fine)
    ; push the stack address
    ; push the flags
    ; push the code segment
    ; push the ip

    ; lets access the struct passed to us
    mov ebx, [ebp+4]
    ; push the data segment
    push dword[ebx+44]
    ; push the stack pointer
    push dword[ebx+40]
    ; push the flags
    pushf
    pop eax
    or eax, 0x200 ; in order to enable interupts
    push eax
    ; push the code segment
    push dword[ebx+32]
    ; push the ip to execute
    push dword[ebx+28]
    ; setup segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword[ebx+4]
    call restore_general_purpose_registers
    add esp, 4 ; dont pop to avoid corruption

    ; leave kernel land and go to user land !!!!!
    iretd

; void restore_general_purpose_registers(struct registers* regs); 
restore_general_purpose_registers:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [edx+20]
    mov eax, [edx+24]
    mov ebx, [ebx+12]

    pop ebp
    ret


; void user_registers();
; changes the segment registers to the user data segment register
user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret