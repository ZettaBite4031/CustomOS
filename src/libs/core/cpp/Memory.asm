[bits 32]

; EXPORT void ASMCALL memcpy(void* dst, void* src, size_t size);
global memcpy
memcpy:
    push ebp
    mov ebp, esp

    mov edi, [ebp + 8]  ; dst
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; count
    
    rep movsb

    mov esp, ebp
    pop ebp
    ret

;
; EXPORT void ASMCALL memset(void* dst, uint32_t val, size_t size);
global memset
memset:
    push ebp
    mov ebp, esp

    mov edi, [ebp + 8]  ; dst
    mov eax, [ebp + 12] ; val
    mov ecx, [ebp + 16] ; count

    rep stosb

    mov esp, ebp
    pop ebp
    ret
