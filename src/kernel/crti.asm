section .init
global _init
_init:
    push ebp 
    mov ebp, esp
    ; gcc will place the contents of crtbegin.o's .init section here. 
    ; Essentially finishing the function for us

section .fini
global _fini
_fini:
    push ebp
    mov ebp, esp
    ; gcc does the same here, but with the .fini section