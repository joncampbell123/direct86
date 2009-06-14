; text display code (part of the BIOS that handles the VGA hardware)

clearscreen:            push        es
                        push        cx
                        push        ax
                        push        di
                        mov         cx,0B800h
                        mov         es,cx
                        mov         cx,80*25
                        mov         ax,0720h
                        xor         di,di
                        cld
                        rep         stosw
                        pop         di
                        pop         ax
                        pop         cx
                        pop         es
                        ret

setpalette:             push        si
                        push        dx
                        push        ax
                        push        cx
                        mov         cx,768
                        mov         dx,3C8h
                        mov         al,0
                        out         dx,al
                        inc         dx
setpalettel1:           lodsb
                        shr         al,1
                        shr         al,1
                        out         dx,al
                        loop        setpalettel1
                        pop         cx
                        pop         ax
                        pop         dx
                        pop         si
                        ret

updatecursorpos:        push        ds
                        push        si
                        push        dx
                        push        cx
                        push        ax
                        xor         si,si
                        mov         ds,si
                        xor         ah,ah
                        mov         al,[451h]
                        mov         dx,80
                        mul         dx
                        mov         cx,ax
                        xor         ah,ah
                        mov         al,[450h]
                        add         cx,ax
                        mov         dx,3D4h
                        mov         al,0Eh
                        mov         ah,ch
                        out         dx,ax
                        mov         al,0Fh
                        mov         ah,cl
                        out         dx,ax
                        pop         ax
                        pop         cx
                        pop         dx
                        pop         si
                        pop         ds
                        ret

videoinit:              call        clearscreen
                        mov         ax,cs
                        mov         ds,ax
                        mov         si,vid_default_palette
                        call        setpalette
; init data area
                        mov         ax,40h
                        mov         ds,ax
                        mov         es,ax
                        mov         [49h],BYTE 3
                        mov         [4Ah],WORD 80
                        xor         ax,ax
                        mov         di,50h
                        mov         cx,8
                        rep         stosw
; make our INT 10h availaible
                        mov         ds,ax
                        mov         [40h],WORD int_10
                        mov         [42h],WORD 0F000h
                        ret

; the INT 10h handler

int_10:                 cmp         ah,0Eh
                        je          int_10_0E
                        cmp         ah,02h
                        je          int_10_02
                        cmp         ah,03h
                        je          int_10_03
						cmp			ah,0Fh
						je			int_10_0F
                        cmp         ah,00h
                        je          int_10_00
                        db          0EAh
                        dw          0AB10h
                        dw          0F000h
int_10_end:             iret
int_10_00:              push        ax
                        push        dx
                        mov         ah,al
                        mov         al,88h      ; CRTC reg 88h
                        mov         dx,3D4h
                        out         dx,ax
                        pop         dx
                        pop         ax
                        jmp         int_10_end
int_10_0F:              push        ds
                        push        si
                        xor         ax,ax
                        mov         ds,ax
                        mov         si,ax
                        mov         ah,[44Ah]   ; columns
                        mov         al,[449h]   ; display mode
                        mov         bh,[462h]   ; active page
                        pop         si
                        pop         ds
                        jmp         int_10_end
int_10_02:              push        ds
                        push        ax
                        push        si
                        xor         ax,ax
                        mov         ds,ax
                        mov         si,ax
                        mov         [450h],dx
                        pop         si
                        pop         ax
                        pop         ds
                        call        updatecursorpos
                        jmp         int_10_end
int_10_03:              push        ds
                        push        ax
                        push        si
                        xor         ax,ax
                        mov         ds,ax
                        mov         si,ax
						xor			cx,cx
                        mov         dx,[450h]
                        pop         si
                        pop         ax
                        pop         ds
                        jmp         int_10_end
int_10_0E:              push        bp
                        push        es
                        push        ds
                        push        bx
                        push        cx
                        push        dx
                        push        si
                        push        di
                        push        ax
                        xor         ax,ax
                        mov         ds,ax
                        mov         si,ax
                        mov         ax,0B800h
                        mov         es,ax
                        mov         bp,sp
                        mov         ax,[bp]
; control character?
                        cmp         al,7        ; ASCII BEEP?
                        je          int_10_0E_BEEP
                        cmp         al,8        ; ASCII BACKSPACE?
                        je          int_10_0E_BACKSPACE
                        cmp         al,9        ; ASCII TAB?
                        je          int_10_0E_TAB
                        cmp         al,10       ; ASCII LINEFEED?
                        je          int_10_0E_LINEFEED
                        cmp         al,13       ; ASCII CARRIAGE RETURN?
                        je          int_10_0E_CR
; print it
                        push        ax
                        mov         dx,[450h]
                        mov         di,dx
                        and         di,0FFh
                        mov         al,dh
                        xor         ah,ah
                        mov         dx,80
                        mul         dx
                        add         di,ax
                        shl         di,1
                        pop         ax
                        cli
                        mov         [es:di],al
; advance cursor
                        inc         BYTE [450h]
int_10_0E_ADVCHK:       cmp         [450h],BYTE 80
                        jb          int_10_0E_end
                        mov         [450h],BYTE 0
int_10_0E_LINEFEED:     cmp         [451h],BYTE 24
                        jae         int_10_0E_advance_scroll
                        inc         BYTE [451h]
                        jmp         int_10_0E_end
int_10_0E_BEEP:         jmp         int_10_0E_end
int_10_0E_BACKSPACE:    test        [450h],BYTE 0FFh
                        jz          int_10_0E_end
                        dec         BYTE [450h]
                        jmp         int_10_0E_end
int_10_0E_TAB:          or          [450h],BYTE 7
                        inc         BYTE [450h]
                        jmp         int_10_0E_ADVCHK
int_10_0E_CR:           mov         [450h],BYTE 0
                        jmp         int_10_0E_end

int_10_0E_advance_scroll:
;                        mov         ax,es
;                        mov         ds,ax
;                        mov         cx,80*24
;                        xor         di,di
;                        mov         si,160
;                        cld
;                        rep         movsw
; clear bottom row
;                        mov         ax,0720h
;                        mov         cx,80
;                        rep         stosw
                        push        ax
                        push        dx
                        mov         ax,0189h
                        mov         dx,3D4h
                        out         dx,ax       ; use "kludge" extensions
                        pop         dx
                        pop         ax
                        jmp         int_10_0E_end

; end of INT 10h AH=0Eh
int_10_0E_end:          call        updatecursorpos
                        pop         ax
                        pop         di
                        pop         si
                        pop         dx
                        pop         cx
                        pop         bx
                        pop         ds
                        pop         es
                        pop         bp
                        jmp         int_10_end

puts:                   push        si
                        push        ax
                        push        bx
putsl:                  lodsb
                        or          al,al
                        jz          putse
                        mov         ah,0Eh
                        xor         bx,bx
                        int         10h
                        jmp         putsl
putse:                  pop         bx
                        pop         ax
                        pop         si
                        ret

putc:                   push        ax
                        push        bx
                        mov         ah,0Eh
                        xor         bx,bx
                        int         10h
                        pop         bx
                        pop         ax
                        ret

puthex:                 push        ax
                        and         al,0Fh
                        cmp         al,0Ah
                        jae         puthexa
                        add         al,'0'
                        jmp         puthexc
puthexa:                add         al,('A' - 10)
puthexc:                call        putc
                        pop         ax
                        ret

puthex8:                push        ax
                        push        cx
                        mov         cl,4
                        ror         al,cl
                        call        puthex
                        ror         al,cl
                        call        puthex
                        pop         cx
                        pop         ax
                        ret

vid_fnt_chr8x8:
incbin "video/chr8x8.set"

vid_fnt_chr8x16:
incbin "video/chr8x16.set"

vid_default_palette:
incbin "video/stdvga.pal"
