[bits 32]
extern i686_ISR_Handler 

%macro ISR_NOERRORCODE 1
global i686_ISR%1
i686_ISR%1:
    push 0
    push %1
    jmp ISR_Common
%endmacro

%macro ISR_ERRORCODE 1
global i686_ISR%1
i686_ISR%1:
    push %1
    jmp ISR_Common
%endmacro


%include "arch/i686/isrs_gen.inc"


ISR_Common:
    pusha
    
    xor eax, eax
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call i686_ISR_Handler

    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret