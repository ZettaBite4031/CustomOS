[bits 32]

;
;void EXTERN i686_IDT_Load(IDTDescriptor* desc);
;
global i686_IDT_Load
i686_IDT_Load:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret