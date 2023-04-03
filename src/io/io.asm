section .asm

global insb
global insw
global outb
global outw

; first argument port to read from
insb:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8] ; port to read from
    in al, dx ; ax contains the return value from call
    
    pop ebp
    ret

; first argument port to read from
insw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8] ; port to read from
    in ax, dx
    
    pop ebp
    ret

; first argument port to write to
; second argument value to write
outb:
    push ebp
    mov ebp, esp

    xor eax, eax 
    mov eax, [ebp + 12] ; data
    mov edx, [ebp + 8] ; port
    out dx, al ; ax contains the return value from call
    
    pop ebp
    ret

; first argument port to write to
; second argument value to write
outw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov eax, [ebp + 12] ; data
    mov edx, [ebp + 8] ; port
    out dx, ax
    
    pop ebp
    ret