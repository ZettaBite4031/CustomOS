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