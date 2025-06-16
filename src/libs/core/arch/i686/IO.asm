[bits 32]

; EXPORT void ASMCALL OutB(uint16_t port, uint8_t value);
global OutPortB
OutPortB:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret


; EXPORT uint8_t ASMCALL InB(uint16_t port);
global InPortB
InPortB:
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

; EXPORT void ASMCALL OutPortW(uint16_t port, uint16_t value);
global OutPortW
OutPortW:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

; EXPORT uint16_t ASMCALL InPortW(uint16_t port);
global InPortW
InPortW:
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

; EXPORT void ASMCALL OutPortL(uint16_t port, uint32_t value);
global OutPortL
OutPortL:
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

; EXPORT uint32_t ASMCALL InPortL(uint16_t port);
global InPortL
InPortL:
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

; EXPORT void ASMCALL PANIC();
global PANIC
PANIC:
    cli
    hlt