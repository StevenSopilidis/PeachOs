[BITS 32]

section .asm

global print:function
global peachos_getKey:function
global peachos_free:function
global peachos_malloc:function
global peachosPutChar:function

; void print(const char* filename)
print:
    push ebp
    mov ebp, esp
    push dword[ebp+8]
    mov eax, 1 ; Command print
    int 0x80
    add esp, 4
    pop ebp
    ret

; int getkey()
peachos_getKey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; Command getKey
    int 0x80
    pop ebp
    ret


; void peachosPutChar(char c)
peachosPutChar:
    push ebp
    mov ebp, esp
    mov eax, 3 
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

; void* peachos_malloc(size_t size)
peachos_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; Command malloc (Allocates memory for the process)
    push dword[ebp+8] ; Variable "size"
    int 0x80
    add esp, 4
    pop ebp
    ret

; void peachos_free(void* ptr)
peachos_free:
    push ebp
    mov ebp, esp
    mov eax, 5 ; Command 5 free (Frees the allocated memory for this process)
    push dword[ebp+8] ; Variable "ptr"
    int 0x80
    add esp, 4
    pop ebp
    ret