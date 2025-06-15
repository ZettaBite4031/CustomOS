[bits 32]

;
; EXPORT void ASMCALL memcpy(void* dst, void* src, size_t size);
; Args (cdecl):
;  [ebp + 8]  - dst
;  [ebp + 12]  - src
;  [ebp + 16] - size
; Returns:
;  eax - dst
;
global memcpy
memcpy:
    push ebp
    mov ebp, esp

    mov edi, [ebp + 8]  ; dst
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; count

    push edi ; save return value (dst)

    ; check for zero length
    test ecx, ecx
    je .done

    cld
    rep movsb

    .done:
    pop eax
    pop ebp
    ret

;
; EXPORT void ASMCALL memset(void* dst, uint32_t val, size_t size);
; Args (cdecl):
;  [ebp + 8]  - dst
;  [ebp + 12]  - val
;  [ebp + 16] - size
; Returns:
;  eax - dst
;
global memset
memset:
    push ebp
    mov ebp, esp

    mov edi, [ebp + 8]  ; dst
    mov eax, [ebp + 12] ; value
    mov ecx, [ebp + 16] ; count

    push edi ; save return value (dst)

    ; check for zero length
    test ecx, ecx
    je .done

    cld
    rep stosb

    .done:
    pop eax
    pop ebp
    ret

;
; EXPORT int ASMCALL memcmp(const void* s1, const void* s2, size_t size)
; Args (cdecl):
;   [ebp + 8]  - s1  
;   [ebp + 12] - s2  
;   [ebp + 16] - size 
; Returns:
;   eax = 0 if equal
;   eax < 0 if s1 < s2
;   eax > 0 if s1 > s2
;
global memcmp
memcmp:
    push ebp
    mov ebp, esp

    mov esi, [ebp + 8]    ; s1
    mov edi, [ebp + 12]   ; s2
    mov ecx, [ebp + 16]   ; count
    xor eax, eax          ; assume equal (return 0)

    test ecx, ecx
    je .done              ; length 0 => return 0

    cld                   ; clear direction flag (forward compare)
    repe cmpsb            ; compare while equal and ecx > 0

    je .done              ; all bytes matched

    ; Mismatch occurred, need to compute s1[i] - s2[i]
    movzx eax, byte [esi - 1]
    movzx ebx, byte [edi - 1]
    sub eax, ebx

.done:
    pop ebp
    ret