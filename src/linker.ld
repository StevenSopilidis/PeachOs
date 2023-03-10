ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS 
{
    . = 1M; /* our kernel will be loaded into 1 MB of memory (location=0x100000)*/
    .text : 
    {
        *(.text)
    }

    .rodata : 
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}