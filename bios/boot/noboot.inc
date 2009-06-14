; this code makes up the ROM routine that displays it's message when no boot
; is availaible.

noboot:     sti
            mov     si,nobootstr
            xor     ax,ax
            sub     ax,1000h
            mov     ds,ax
noboot_l1:  lodsb
            or      al,al
            jz      noboot_2
            mov     ah,0Eh
            xor     bx,bx
            int     10h
            jmp     noboot_l1
noboot_2:   mov     ah,0
            int     16h
            nop
            nop
            jmp     Int19routine

nobootstr   db      'Unable to load a boot routine',13,10
            db      'Insert a virtual disk and press any key to continue',13,10
            db      13,10,0

