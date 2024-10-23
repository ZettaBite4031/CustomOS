bits 16

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
ZOS_START:
    ; move partition entry from MBR to prevent overwriting
    mov ax, PARTITION_ENTRY_SEGMENT
    mov es, ax
    mov di, PARTITION_ENTRY_OFFSET
    mov cx, 16
    rep movsb

    mov ax, 0
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7C00

    push es
    push word .after
    retf
    .after:

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
    ; load stage2
    mov si, stage2_location
    
    mov ax, STAGE2_LOAD_SEGMENT
    mov es, ax

    mov bx, STAGE2_LOAD_OFFSET

    .read_loop:
        mov eax, [si]
        add si, 4
        mov cl, [si]
        inc si

        cmp eax, 0
        je .read_finish

        call ReadDisk

        xor ch, ch
        shl cx, 5
        mov di, es
        add di, cx
        mov es, di

        jmp .read_loop

    .read_finish:
    mov dl, [ebr_drive_number]
    mov di, PARTITION_ENTRY_SEGMENT
    mov si, PARTITION_ENTRY_OFFSET

    mov ax, STAGE2_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET


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

    Stage2NotFound:
        mov si, Stage2NotFoundMSG
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
        mov [extensions_dap.count], cl

        mov ah, 0x42
        mov si, extensions_dap
        mov di, 3
        jmp .retry

        .no_disk_extensions:
        push cx
        call LBS2CHS
        pop ax
        
        mov ah, 02h
        mov di, 3
        .retry:
            pusha
            stc
            int 13h
            popa
            jnc .done

            ; failed
            call DiskReset

            dec di
            test di, di
            jnz .retry

        .fail:
            jmp FloppyError
        
        .done:    
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
    Stage2NotFoundMSG: db "stage2.bin not found", ENDL, 0
    FileStage2Bin: db "STAGE2  BIN"

section .data
    has_extensions: db 0
    extensions_dap: 
        .size:      db 10h
                    db 0h
        .count:     dw 0
        .offset:    dw 0
        .segment:   dw 0
        .lba:       dq 0


    STAGE2_LOAD_SEGMENT equ 0x0
    STAGE2_LOAD_OFFSET equ 0x500

    PARTITION_ENTRY_SEGMENT equ 0x2000
    PARTITION_ENTRY_OFFSET  equ 0x0

    global stage2_location
    stage2_location: times 30 db 0


section .bss
    Buffer: resb 512
