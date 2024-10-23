%macro x86_EnterRealMode 0
    [bits 32]
    jmp word 18h:.PMode16 ; 1 - jump to 16-bit protected mode

    .PMode16:
    [bits 16]
    ; 2 - disable protected mode bit in CR0
    mov eax, cr0
    and al, ~1
    mov cr0, eax

    ; 3 - jump to real mode
    jmp word 00h:.RMode

    .RMode:
    ; 4 - setup segments
    mov ax, 0
    mov ds, ax
    mov ss, ax

    ; 5 - enable interrupts
    sti
%endmacro

%macro x86_EnterProtectedMode 0
    [bits 16]
    ; 1 - disable interrupts 
    cli
    ; 2 & 3 - Enable A20 gate and load GDT (already done)
    ; 4 - set protection enable flag in CR0
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; 5 - far jump into protected mode
    jmp dword 08h:.PMode

    .PMode:
    [bits 32]
    ; 6 - setup segment
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
%endmacro

;
; Convert linear address to segment:offset address (SSSS:OOOO)
; Args:
;   1 - (in)  linear address
;   2 - (out) target segment (e.g. es)
;   3 - target 32-bit register (e.g. eax)
;   4 - target lower 16-bit half of #3 (e.g. ax)
;
%macro LinearToSegOff 4
    mov %3, %1      ; linear address to eax
    shr %3, 4
    mov %2, %4
    mov %3, %1
    and %3, 0xF
%endmacro

global x86_OutB
x86_OutB:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global x86_InB
x86_InB:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret


;
;bool __attribute__((_cdecl)) x86_DiskReset(uint8_t drive);
;
global x86_DiskReset
x86_DiskReset:
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    mov ah, 0
    mov dl, [bp + 8]
    stc
    int 13h

    mov eax, 1
    sbb eax, 0
    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret
; 
;bool __attribute__((_cdecl)) x86_DiskRead(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t* lowerDataOut);
;
global x86_DiskRead
x86_DiskRead:
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push ebx
    push es

    mov dl, [bp + 8]        ; drive

    mov ch, [bp + 12]       ; cylinder
    mov cl, [bp + 13]
    shl cl, 6

    mov al, [bp + 16]       ; secotrs
    and al, 3Fh
    or cl, al

    mov dh, [bp + 20]       ; heads

    mov al, [bp + 24]       ; count

    ; lowerDataOut
    LinearToSegOff [bp + 28], es, ebx, bx

    mov ah, 02h
    stc
    int 13h

    mov eax, 1
    sbb eax, 0

    pop es
    pop ebx

    push eax

    x86_EnterProtectedMode

    pop eax

    mov esp, ebp
    pop ebp
    ret
; 
;bool __attribute__((_cdecl)) x86_GetDiskDriveParams(uint8_t drive, uint8_t* driveType, uint16_t* cylinders, uint16_t* sectors, uint16_t* heads);
; 
global x86_GetDiskDriveParams
x86_GetDiskDriveParams:
    push ebp
    mov ebp, esp
    
    x86_EnterRealMode

    [bits 16]
    push es
    push bx
    push esi
    push di

    mov dl, [bp + 8]
    mov ah, 08h
    mov di, 0
    mov es, di
    stc
    int 13h

    ; out params

    ; drive type from bl
    LinearToSegOff [bp + 12], es, esi, si
    mov es:[si], bl

    ; cylinders 
    mov bl, ch
    mov bh, cl
    shr bh, 6
    inc bx
    LinearToSegOff [bp + 16], es, esi, si
    mov es:[si], bx

    ; sectors
    xor ch, ch
    and cl, 3Fh
    LinearToSegOff [bp + 20], es, esi, si
    mov es:[si], cx

    ; heads
    mov cl, dh
    inc cx
    LinearToSegOff [bp + 24], es, esi, si
    mov es:[si], cx

    push di
    push esi
    push bx
    push es

    ; return value
    mov eax, 1
    sbb eax, 0
    push eax

    x86_EnterProtectedMode
    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret
    
global x86_Video_GetVbeInfo
x86_Video_GetVbeInfo:
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push edi
    push es

    LinearToSegOff [bp + 8], es, edi, di
    mov ax, 0x4F00
    int 10h

    push eax
    x86_EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Video_GetModeInfo
x86_Video_GetModeInfo:
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push edi
    push es
    push ebp    ; certain bios' could change ebp (e.g. bochs')
    push ecx

    mov ax, 0x4F01
    mov cx, [bp + 8]
    LinearToSegOff [bp + 12], es, edi, di
    int 10h

    pop ecx
    pop ebp
    pop es
    pop edi

    push eax
    x86_EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Video_SetMode
x86_Video_SetMode:
    push ebp
    mov ebp, esp

    x86_EnterRealMode

    push edi
    push es
    push ebp    ; certain bios' could change ebp (e.g. bochs')
    push ebx

    mov ax, 0
    mov es, ax
    mov edi, 0
    mov ax, 0x4F02
    mov bx, [bp + 8]
    int 10h
    
    pop ebx
    pop ebp
    pop es
    pop edi

    push eax
    x86_EnterProtectedMode
    pop eax

    mov esp, ebp
    pop ebp
    ret
