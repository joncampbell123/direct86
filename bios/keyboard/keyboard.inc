; keyboard code

; THIS ADDRESS IS IMPORTANT:

EMU_INT15_SEG       EQU         0F000h
EMU_INT15_OFS       EQU         0AB15h

keyboard_init:      xor         ax,ax
                    mov         ds,ax
; patch INT 9h (IRQ 1) through our routine so we can handle the keyboard
                    mov         [9h * 4],WORD bios_irq1
                    mov         [(9h * 4) + 2],WORD 0F000h
; patch INT 15h through our routine so we can add extra services
; (i.e. the INT 15h AH=4Fh keyboard intercept)
                    mov         [15h * 4],WORD bios_int15
                    mov         [(15h * 4) + 2],WORD 0F000h
; clear the keyboard buffer mechanism
                    mov         [41Ah],WORD 0x1E
                    mov         [41Ch],WORD 0x1E
                    ret

; BIOS IRQ #1 keyboard handler
bios_irq1:          cli
                    push        bx
                    push        dx
                    push        ds
                    push        es
                    push        si
                    push        di
                    push        ax
                    in          al,64h
                    test        al,1
                    jnz         bios_irq1_nj0           ; stupid -128 to 127 short conditional jumps!
                    jmp         bios_irq1_end
bios_irq1_nj0:      in          al,60h
; now.... the INT 15H AH=4Fh feature many BIOSes carry today!
                    mov         ah,4Fh
                    stc
                    int         15h
                    jc          bios_irq1_nj1           ; carry flag clear if the intercept routine insists on it being ignored
                    jmp         bios_irq1_end
; okay, now we process it!
bios_irq1_nj1:      mov         si,0F000h
                    mov         ds,si
                    mov         si,40h
                    mov         es,si
                    xor         si,si
                    mov         dl,al
                    and         dl,80h
                    xor         dl,80h
                    and         al,7Fh

                    cmp         al,3Ah                  ; caps lock?
                    je          bios_irq1_caps
                    cmp         al,45h                  ; num lock?
                    je          bios_irq1_nj2
                    cmp         al,46h                  ; scroll lock?
                    je          bios_irq1_nj2
                    cmp         al,2Ah                  ; left shift?
                    je          bios_irq1_lshift
                    cmp         al,36h                  ; right shift?
                    je          bios_irq1_nj2
                    cmp         al,1Dh                  ; CTRL?
                    je          bios_irq1_nj2
                    cmp         al,38h                  ; ALT?
                    je          bios_irq1_nj2
                    jmp         bios_irq1_nkey
bios_irq1_nj2:      jmp         bios_irq1_end
bios_irq1_lshift:   push        cx
                    mov         cl,6
                    shr         dl,cl
                    and         [es:17h],BYTE (~2) & 0xFF
                    or          [es:17h],dl
                    pop         cx
                    jmp         bios_irq1_end
bios_irq1_caps:     mov         al,[es:18h]
                    shr         dl,1
                    and         [es:18h],BYTE (~0x40) & 0xFF
                    or          [es:18h],dl
                    test        al,40h
                    jnz         bios_irq1_nj2
                    test        dl,40h
                    jz          bios_irq1_nj2
                    xor         [es:17h],BYTE 40h     ; toggle caps lock!
                    jmp         bios_irq1_end
; now, for keys that we only care about if they're being pressed (not released)
bios_irq1_nkey:     test        dl,80h                  ; key down or up?
                    jnz         bios_irq1_nkey_1        ; we don't process key releases!
                    jmp         bios_irq1_end           ; we don't process key releases!
bios_irq1_nkey_1:   push        ax                      ; save the key code
                    mov         bx,key_intransl
                    test        [es:17h],BYTE 3h
                    jz          bios_irq1_trans
                    add         bx,128
bios_irq1_trans:    xlatb
; capitalize it?
                    test        [es:17h],BYTE 40h
                    jz          bios_irq1_stuffit
                    call        ascii_capitalize
bios_irq1_stuffit:  mov         bl,al
                    pop         ax
                    mov         ah,al
                    mov         al,bl
; now to stuff it in the keyboard buffer! (if it's anything)
                    test        ax,-1
                    jz          bios_irq1_end           ; if AX==0, ignore it
                    mov         bx,[es:1Ch]
                    inc         bx
                    and         bl,0FEh
                    cmp         bx,3Eh
                    jb          bios_irq1_stuffchk1
                    cmp         bx,1Eh
                    jae         bios_irq1_stuffchk1
                    mov         bx,1Eh
bios_irq1_stuffchk1:mov         di,[es:1Ah]
                    inc         di
                    and         di,-2
                    mov         [es:1Ah],WORD di
                    cmp         di,3Eh
                    jb          bios_irq1_stuffchk2
                    cmp         di,1Eh
                    jae         bios_irq1_stuffchk2
                    mov         di,1Eh
bios_irq1_stuffchk2:sub         di,2
                    cmp         di,1Eh
                    jae         bios_irq1_stuffchk3
                    mov         di,3Ch
bios_irq1_stuffchk3:cmp         bx,di
                    je          bios_irq1_stuffed
; there's room in the keyboard buffer. put it in!
                    mov         [es:bx],WORD ax
                    add         bx,2
                    cmp         bx,3Eh
                    jb          bios_irq1_stuffchk4
                    mov         bx,1Eh
bios_irq1_stuffchk4:mov         [es:1Ch],bx
                    jmp         bios_irq1_end

; no room!!!
bios_irq1_stuffed:  jmp         bios_irq1_end

bios_irq1_end:      mov         al,20h
                    out         20h,al
                    mov         al,61h
                    out         20h,al
                    pop         ax
                    pop         di
                    pop         si
                    pop         es
                    pop         ds
                    pop         dx
                    pop         bx
                    iret

bios_int15:         cmp         ah,4Fh
                    je          bios_int15_4F
bios_int15_emu:     db          0EAh            ; JMP
                    dw          EMU_INT15_OFS
                    dw          EMU_INT15_SEG
bios_int15_4F:      stc                         ; let's not ignore any keypresses!
bios_int15_end:     iret

; key scan code to ASCII translation tables
key_intransl            db            0,  27, '1', '2', '3', '4', '5', '6'
                        db          '7', '8', '9', '0', '-', '=',   8,   9
                        db          'q', 'w', 'e', 'r', 't', 'y', 'u', 'i'
                        db          'o', 'p', '[', ']',  13,   0, 'a', 's'
                        db          'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'
                        db           39, '`',   0, '\', 'z', 'x', 'c', 'v'
                        db          'b', 'n', 'm', ',', '.', '/',   0, '*'
                        db            0, ' ',   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0, '7'
                        db          '8', '9', '-', '4', '5', '6', '+', '1'
                        db          '2', '3', '0', '.',   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
; shifted
                        db            0,   0, '!', '@', '#', '$', '%', '^'
                        db          '&', '*', '(', ')', '_', '+',   0,   0
                        db          'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I'
                        db          'O', 'P', '{', '}',   0,   0, 'A', 'S'
                        db          'D', 'F', 'G', 'H', 'J', 'K', 'L', ':'
                        db           34, '~',   0, '|', 'Z', 'X', 'C', 'V'
                        db          'B', 'N', 'M', '<', '>', '?',   0, '*'
                        db            0, ' ',   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0, '-',   0,   0,   0, '+',   0
                        db            0,   0,   0, '.',   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
; control
                        db            0,  27,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   8,   9
                        db           17,  23,   5,  18,  20,  25,  21,   9
                        db           15,  16,  27,  29,  13,   0,   1,  19
                        db            4,   6,   7,   8,  10,  11,  12,   0
                        db           39,   0,   0,  28,  26,  24,   3,  22
                        db            2,  14,  13,   0,   0,   0,   0,   0
                        db            0, ' ',   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,  31,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
; control-shifted
                        db            0,  27,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   8,   9
                        db           17,  23,   5,  18,  20,  25,  21,   9
                        db           15,  16,  27,  29,  13,   0,   1,  19
                        db            4,   6,   7,   8,  10,  11,  12,   0
                        db           39,   0,   0,  28,  26,  24,   3,  22
                        db            2,  14,  13,   0,   0,   0,   0,   0
                        db            0, ' ',   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0
                        db            0,   0,   0,   0,   0,   0,   0,   0

ascii_capitalize:       cmp     al,'a'
                        jb      ascii_capitalize_end
                        cmp     al,'z'
                        ja      ascii_capitalize_end
                        sub     al,32
ascii_capitalize_end:   ret

