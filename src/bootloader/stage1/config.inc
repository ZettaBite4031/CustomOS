%ifndef _CONFIG__INC_
%define _CONFIG__INC_


; stage1 will relocate to below address
; make sure to leave room for the stack which grows downwards
; don't forget to update the offset in linker.ld as well
STAGE1_SEGMENT              equ 0x0
STAGE1_OFFSET               equ 0x700


%endif