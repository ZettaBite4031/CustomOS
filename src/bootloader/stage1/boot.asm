bits 16

%include "config.inc"

%define ENDL 0x0A, 0x0D

%define fat12 1
%define fat16 2
%define fat32 3
%define ext2  4

;
; FAT HEADERS
;

section .fsjump
    jmp short ZOS_START
    nop

section .fsheaders
%if (FILESYSTEM == fat12) || (FILESYSTEM == fat16) || (FILESYSTEM == fat32)
    bdb_oem:                    db "MSWIN4.1"
    bdb_bytes_per_sector:       dw 512
    bdb_sectors_per_cluster:    db 1
    bdb_reserved_sectors:       dw 1
    bdb_fat_count:              db 2
    bdb_dir_entries_count:      dw 0E0h
    bdb_total_sectors:          dw 2880
    bdb_media_descriptor_type:  db 0F0h
    bdb_sectors_per_fat:        dw 9
    bdb_sectors_per_track:      dw 18
    bdb_heads:                  dw 2
    bdb_hidden_sectors:         dd 0
    bdb_large_sector_count:     dd 0

%if (FILESYSTEM == fat32)
    fat32_sectors_per_fat:      dd 0
    fat32_flags:                dw 0
    fat32_version:              dw 0
    fat32_rootdir_cluster:      dd 0
    fat32_fsinfo_sector:        dw 0
    fat32_backup_bootsector:    dw 0
    fat32_reserved:             times 12 db 0
%endif

    ; extended boot record
    ebr_drive_number:           db 0
    fat32_NTFlags:              db 0            ; flags for NTFS. Reserved otherwise
    ebr_signature:              db 29h
    ebr_volume_id:              db 04h, 00h, 03h, 01h
    ebr_volume_label:           db "ZETTABITEOS"
    ebr_system_id:              times 8 db 0
%endif

section .entry
global ZOS_START

    ; the MBR will pass the following information:
    ; dl - drive number
    ; ds:si - boot partition table entry
    
ZOS_START:
    ; copy partition entry
    mov ax, STAGE1_SEGMENT
    mov es, ax
    mov di, parition_entry
    mov cx, 16
    rep movsb

    ;
    ; relocate stage 1
    ;
    mov ax, 0
    mov ds, ax
    mov si, 0x7C00
    mov di, STAGE1_OFFSET
    mov cx, 512
    rep movsb    

    jmp STAGE1_SEGMENT:.relocated
    .relocated:
    
    ;
    ; INITIALIZATION
    ;

    ; setup data segments
    mov ax, STAGE1_SEGMENT
    mov ds, ax

    ; setup stack
    mov ss, ax
    mov sp, STAGE1_OFFSET

    mov [ebr_drive_number], dl

    ; check extensions present
    stc
    mov ah, 0x41
    mov bx, 0x55AA
    int 13h

    jc .no_disk_extensions
    cmp bx, 0xAA55
    jne .no_disk_extensions

    ; extensions are present
    mov byte [has_extensions], 1
    jmp .load_stage2

    .no_disk_extensions:
    mov byte [has_extensions], 0

    .load_stage2:
    ; load boot table lba
    mov eax, [boot_table_lba]
    mov cx, 1
    mov bx, boot_table
    call ReadDisk

    ;
    ; parse boot table and load
    ;
    mov si, boot_table

    ; parse entry 
    mov eax, [si]
    mov [entry_point], eax
    add si, 4

    .readloop:
        ; read until lba = 0
        cmp dword [si + boot_table_entry.lba], 0
        je .endreadloop

        mov eax, [si + boot_table_entry.lba]
        mov bx, [si + boot_table_entry.load_seg]
        mov es, bx
        mov bx, [si + boot_table_entry.load_off]
        mov cl, [si + boot_table_entry.count]
        call ReadDisk

        add si, boot_table_entry_size
        jmp .readloop

    .endreadloop:
    
    mov dl, [ebr_drive_number]    
    ; es:di points to partition entry
    mov di, parition_entry

    mov ax, [entry_point.seg]
    mov ds, ax

    push ax
    push word [entry_point.off]
    retf


section .text
    WaitKeyReboot:
        mov ah, 0
        int 16h
        jmp 0FFFFh:0

    HALT:
        cli
        hlt
        jmp HALT

;
; ERRORS
;
    FloppyError:
        mov si, ReadFailedMSG
        call puts
        jmp WaitKeyReboot
;
; FUNCTIONS
;

;
; Converts an LBA address to a CHS address
; Params:
;   - ax: LBA address
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
;
    LBS2CHS:
        push ax
        push dx

        xor dx, dx                          ; dx = 0
        div word [bdb_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                            ; dx = LBA % SectorsPerTrack

        inc dx                              ; dx = (LBA % SectorsPerTrack) + 1 = sectors
        mov cx, dx                          ; cx = sectors

        xor dx, dx                          ; dx = 0
        div word [bdb_heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                            ; dx = (LBA / SectorsPerTrack) % Heads = head
        mov dh, dl                          ; dl = head
        mov ch, al                          ; ch = cylinder (low 8 bits)
        shl ah, 6
        or cl, ah                           ; put high 2 bits of cylinder in CL

        pop ax
        mov dl, al
        pop ax
        ret 

;
; Reads sectors from disk
; Params:
;   - eax: LBA address
;   - cl: number of sectors to read (max 128)
;   - dl: drive number
;   - es:bx: memory address to store data
;
    ReadDisk:
        push eax
        push bx
        push cx
        push dx
        push si
        push di

        cmp byte [has_extensions], 1
        jne .no_disk_extensions
        ; with extensions
        mov [extensions_dap.lba], eax
        mov [extensions_dap.segment], es
        mov [extensions_dap.offset], bx
        mov [extensions_dap.count], cx

        mov ah, 0x42
        mov si, extensions_dap
        mov di, 3
        .retry:
            pusha
            stc
            int 13h
            popa
            jnc .function_end

            ; failed
            call DiskReset

            dec di
            test di, di
            jnz .retry

        .fail:
            jmp FloppyError


        .no_disk_extensions:
        mov esi, eax                        ; save lba to esi 
        mov di, cx                          ; save sector count to di

        .outer_loop:
            mov eax, esi
            call LBS2CHS
            mov ax, 1                       ; read one sector
            
            push di
            mov di, 3
            mov ah, 02h
            .inner_retry:
                pusha
                stc
                int 13h
                popa
                jnc .inner_done

                ; failed
                call DiskReset

                dec di
                test di, di
                jnz .inner_retry

            .inner_fail:
                jmp FloppyError
            
            .inner_done:    
            pop di

            cmp di, 0
            je .function_end

            inc esi
            dec di
            mov ax, es
            add ax, 512 / 16
            mov es, ax
            jmp .outer_loop

        .function_end:

        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop eax
        ret


;
; Resets disk controller
; Params:
;   - dl: drive number
;
    DiskReset:
        pusha 
        mov ah, 0
        stc
        int 13h
        jc FloppyError
        popa 
        ret

;
; Prints a string to the scree
; Params:
;  - ds:si : points to a string
;
    puts:
        push si
        push ax

        .loop:
            lodsb
            or al, al
            jz .done

            mov ah, 0x0E
            int 0x10

            jmp .loop

        .done:
        pop ax
        pop si
        ret


;
; STRINGS
;
section .rodata
    ReadFailedMSG: db "Failed read!", ENDL, 0

section .data
    extensions_dap: 
        .size:      db 10h
                    db 0
        .count:     dw 0
        .offset:    dw 0
        .segment:   dw 0
        .lba:       dq 0

    struc boot_table_entry
        .lba        resd 1
        .load_off   resw 1
        .load_seg   resw 1
        .count      resw 1
    endstruc

    global boot_table_lba
    boot_table_lba: dd 0

section .bss
    has_extensions:     resb 1
    Buffer:             resb 512
    parition_entry:     resb 16
    boot_table:         resb 512
    entry_point:
        .off:           resw 1
        .seg:           resw 1