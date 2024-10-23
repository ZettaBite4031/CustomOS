[bits 32]

; EXPORT void ASMCALL OutB(uint16_t port, uint8_t value);
global OutPort
OutPort:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret


; EXPORT uint8_t ASMCALL InB(uint16_t port);
global InPort
InPort:
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

; EXPORT void ASMCALL PANIC();
global PANIC
PANIC:
    cli
    hlt