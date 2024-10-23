section .init
    pop ebp
    ret
    ; gcc will place the contents of crtend.o's .init section here. 
    ; Essentially finishing the function for us

section .fini
    pop ebp
    ret
    ; gcc does the same here, but with the .fini section
