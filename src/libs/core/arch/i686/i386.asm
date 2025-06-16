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

; EXPORT void ASMCALL DisableInterrupts();
global DisableInterrupts
DisableInterrupts:
    cli 
    ret

; EXPORT void ASMCALL EnableInterrupts();
global EnableInterrupts
EnableInterrupts:
    sti 
    ret

; EXPORT void ASMCALL PANIC();
global PANIC
PANIC:
    cli
    hlt

; EXPORT void ASMCALL LoadGDT(void* desc, uint16_t cs, uint16_t ds);
global LoadGDT
LoadGDT:
    push ebp
    mov ebp, esp

    ; load GDT
    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .ReloadCS
    retf
    .ReloadCS:

    ; reload data segment
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, ebp
    pop ebp
    ret

 
; EXPORT void ASMCALL LoadIDT(void* desc);
global LoadIDT
LoadIDT:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret


; EXPORT void ASMCALL IOWait();
global IOWait
IOWait:
    mov dx, 0x80
    mov al, 0x00
    out dx, al
    ret

; --------------------------------------------------------------

extern ISRSHandler

%macro ISR_NOERRORCODE 1
global ISRWrapper%1
ISRWrapper%1:
    push 0
    push %1
    jmp ISR_Common
%endmacro

%macro ISR_ERRORCODE 1
global ISRWrapper%1
ISRWrapper%1:
    push %1
    jmp ISR_Common
%endmacro

%include "arch/i686/genISRS.inc"

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
    call ISRSHandler

    add esp, 4

    pop eax,
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa

    add esp, 8

    iret