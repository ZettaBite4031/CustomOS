[BITS 32]

global load_cr3
global enable_paging
global invlpg

section .text

; void load_cr3(uint32_t page_directory_phys)
load_cr3: 
    mov eax, [esp + 4]
    mov cr3, eax
    ret

; void enable_paging(void)
enable_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

; void invlpg(void*)
invlpg:
    mov eax, [esp + 4]
    invlpg [eax]
    ret