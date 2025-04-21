global i686_OutB
i686_OutB:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret


global i686_InB
i686_InB:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

;void EXTERN i686_OutW(uint16_t port, uint16_t value);
global i686_OutW
i686_OutW:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

;uint16_t EXTERN i686_InW(uint16_t port);
global i686_InW
i686_InW:
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

;void EXTERN i686_OutL(uint16_t port, uint32_t value);
global i686_OutL
i686_OutL:
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

;uint32_t EXTERN i686_InL(uint16_t port);
global i686_InL
i686_InL:
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

;void EXTERN i686_EnableInterrupts();
global i686_EnableInterrupts
i686_EnableInterrupts:
    sti
    ret


;void EXTERN i686_DisableInterrupts();
global i686_DisableInterrupts
i686_DisableInterrupts:
    cli 
    ret


; void EXTERN i686_PANIC();
global i686_PANIC
i686_PANIC:
    cli
    hlt


; void EXTERN CRASH();
global CRASH
CRASH:
    ; purposefully divide by zero to crash
    mov eax, 0
    div eax
    ret