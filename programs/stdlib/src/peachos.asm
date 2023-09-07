[BITS 32]

section .asm

global print:function
global getKey:function
global peachosMalloc:function
global peachosFree:function

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
getKey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; Command getKey
    int 0x80
    pop ebp
    ret

; void* peachos_malloc(size_t size)
peachosMalloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; command malloc
    push dword[ebp+8] ; argument for size of memory
    int 0x80
    add esp, 4
    pop ebp
    ret

; void peachos_free(void* ptr)
peachosFree:
    push ebp
    mov ebp, esp
    mov eax, 5 ; command free
    push dword[ebp+8] ; ptr to free
    int 0x80
    add esp, 4
    pop ebp
    ret