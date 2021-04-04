; WonderSwan Internal ROM Stub
; --------------------------------------
; Version 1.0

; This is a replacement for the internal rom to prevent using/including any copyrighted material.
; It does nothing, just lauching the cartridge.
; It may later include a bootsplah.

%ifdef WONDERSWAN
%define ROM_SIZE 4096
%define ROM_SEG 0FF00h
%else
%define ROM_SIZE 8192
%define ROM_SEG 0FE00h
%endif

; Setup NASM for a 80186
	bits 16
	cpu  186

	org 0000h
    
%define JUMPER_LOCATION 0000h:0400h
; ---------------------------------------------------------------------------------------------
; Boot ROM jumper
; ---------------------------------------------------------------------------------------------
; Lock the bootrom
; Clear a bit of itself
; Jump to FFFF:FFF0
; 
; Must be copied to 0000:0400 then jmp 0000:0400
ram_jumper:
    
    ; Lock the boot rom away
    IN al, 0A0h
    OR al, 001h ; Bit 0 lock the boot rom
    out 0A0h, al
    
    mov ax, cs
    mov es, ax
    mov di, 0400h
    mov cx, (.rm_te_end - ram_jumper)
    xor ax, ax
    xor bx, bx
.rm_te_end:
    rep stosb
    jmp 0FFFFh:00000h
ram_jumper_end:

_start:
    cli
    cld
    push cs
    pop ds
    xor ax, ax
    mov es, ax

    ; Clear the IRAM
    mov     di, 0
    mov     cx, 16384
    xor     al, al
    rep stosb           ; STOSB -> ES:DI

    ; Copy the jumper
    mov     si, ram_jumper
    mov     cx, (ram_jumper_end - ram_jumper)
    mov     di, 0400h
    xor     ax, ax
    mov     es, ax
    rep movsb         ; DS:SI -> ES:DI

    ; Do some register cleanup
    mov     al, 0FFh
    out     0C0h, al
    out     0C1h, al
    out     0C2h, al
    xor     ax, ax
    xor     cx, cx
    xor     dx, dx
    mov     ds, ax
    mov     es, ax

    ; Jump to the jumper!
    jmp JUMPER_LOCATION
    
%ifdef WONDERSWAN
    db "WonderSwan"
%elifdef WONDERSWANCOLOR
    db "WonderSwan Color"
%elifdef SWANCRYSTAL
    db "WonderSwan Crystal"
%else
%endif
    db " internal ROM Stub for NewOswan (c)2021 986-Studio"
; Create at the end of the block, add padding if needed
TIMES (ROM_SIZE - 16) - ($-$$) DB 0FFh

    jmp ROM_SEG:_start              ; 0
    db 00        ; ??               ; 5
    db 0F0h      ; Dev ID           ; 6
    db 00        ; Min Swan type    ; 7
    db 00000100b ; flags            ; C
    db 01        ; No RTC           ; D
    dw 0FFFFh    ; CRC (need to update it after, but as not used by the rom, no need for now)