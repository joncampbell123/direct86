; This code is executed on reset.

reset:                  cli
; set up regs
                        xor     ax,ax
                        mov     ds,ax
                        mov     es,ax
                        mov     ss,ax
; set up stack in conventional RAM (if any)
; if not enough conventional RAM to be comforatble, give up and display err msg
                        mov     di,0FFFCh       ; 64KB limit?
reset_sp_1:             mov     WORD [di],5AB2h
                        cmp     WORD [di],5AB2h
                        jnz     reset_sp_1b     ; stupid 80x86 short conditional jumps...
                        jmp     reset_sp_works
reset_sp_1b:            sub     di,4000h        ; jump back 16KB
                        cmp     di,7FFCh        ; must be > 32KB
                        jb      reset_sp_no_ram
                        jmp     reset_sp_1
; no conventional RAM? let user know!
reset_sp_no_ram:        mov     ax,0388h
                        mov     dx,3D4h
                        out     dx,ax           ; make sure text mode
                        mov     ax,cs
                        mov     ds,ax
                        mov     ax,0B800h
                        mov     es,ax
                        xor     di,di
                        mov     si,reset_no_ram_errs1
                        mov     cx,80
                        mov     ah,0Ch
                        cld
reset_sp_no_ram_s1:     lodsb
                        or      al,al
                        jz      reset_sp_no_ram_s1b
                        stosw
                        loop    reset_sp_no_ram_s1
reset_sp_no_ram_s1b:    mov     al,20h
                        rep     stosw
                        mov     si,reset_no_ram_errs2
                        mov     cx,80
reset_sp_no_ram_s2:     lodsb
                        or      al,al
                        jz      reset_sp_no_ram_s2b
                        stosw
                        loop    reset_sp_no_ram_s2
reset_sp_no_ram_s2b:    mov     al,20h
                        rep     stosw
                        jmp     short $

reset_no_ram_errs1      db      'INSUFFICIENT LOWER RAM!!!',0
reset_no_ram_errs2      db      'This ROM BIOS requires that at least 32KB of lower RAM exist in this machine.',0

reset_sp_works:         cmp     di,7FFCh        ; we can't have stack at 7FFCh!
                        jne     reset_sp_works_2
                        mov     di,7BFCh        ; put it BEFORE the area of RAM reserved for BOOT
reset_sp_works_2:       mov     sp,di           ; use it!

reset_stage_2:
; count lower RAM
                        call    count_ram
; setup timer
                        call    irq0_init
; initialize VGA hardware
                        call    videoinit
; initialize keyboard
                        call    keyboard_init
; print hello screen
reset_hello:            xor     ax,ax
                        mov     bx,0F000h
                        mov     ds,bx
                        mov     es,ax
                        mov     si,hello_msg
                        call    puts
; offer configuration
                        cli
                        mov     si,setup_offer_msg
                        call    puts
                        mov     ah,0
                        int     16h         ; wait for keypress
                        mov     si,setup_offer_msgerase
                        call    puts
                        cmp     ah,53h      ; DELETE key?
                        jne     reset_cont
; configuration!
                        mov     al,1
                        out     0CCh,al     ; set latch to allow writes to BIOS
                        mov     ax,cs
                        mov     ds,ax
                        mov     es,ax
                        mov     ss,ax
                        mov     sp,0BFFEh
                        xor     ax,ax       ; program may expect all regs to be clear
                        xor     bx,bx       ; program may expect all regs to be clear
                        xor     cx,cx       ; program may expect all regs to be clear
                        xor     dx,dx       ; program may expect all regs to be clear
                        xor     si,si       ; program may expect all regs to be clear
                        xor     di,di       ; program may expect all regs to be clear
                        call    config_prog
                        mov     al,0
                        out     0CCh,al     ; clear latch to disallow writes to BIOS
                        jmp     reset
; call our quickboot
reset_cont:             jmp     Int19routine

hello_msg               db      13,10
                        db      'JC-EMU-BIOS v0.54.1',13,10
                        db      '(C) 1998-2001 Jonathan Campbell',13,10
                        db      13,10
                        db      0
setup_offer_msg         db      13,'Press any key to continue or DEL to configure BIOS',13,0
setup_offer_msgerase    db      13,'                                                  ',13,0

