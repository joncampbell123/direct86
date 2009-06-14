; INT 19h service routine source

; address of "temp" var in RAM
INT19_DRV           EQU     3FCh

Int19routine:       cli
                    xor     ax,ax
                    mov     ds,ax
                    mov     ss,ax
                    mov     sp,ax
                    sub     sp,2
                    mov     [INT19_DRV],BYTE 0
Int19routine_l:     xor     ax,ax
                    mov     es,ax
                    mov     ax,201h
                    mov     bx,7C00h
                    mov     cx,0001h
                    xor     dh,dh
                    mov     dl,[INT19_DRV]
                    mov     sp,7BFEh
                    int     13h
                    jc      Int19routine_err
                    jmp     Int19routine_boot
Int19routine_err:   mov     al,[INT19_DRV]
                    inc     al
                    test    al,7Ch
                    mov     [INT19_DRV],al
                    jz      Int19routine_l
                    or      al,7Fh
                    inc     al
                    mov     [INT19_DRV],al
                    jnz     Int19routine_l
                    jmp     noboot
Int19routine_boot:  push    es      ; dont clear registers, a lot of programmers wrote their boot routines assuming BIOS keeps these in the registers
                    pop     ds
                    push    es
                    pop     ss
                    mov     sp,07BFEh
                    jmp     0000h:7C00h

Int19drive          db      0
