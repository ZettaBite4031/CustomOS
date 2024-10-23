[bits 32]

; 
; void EXTERN i686_GDT_Load(GDTDescriptor* desc);
;
global i686_GDT_Load
i686_GDT_Load:
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