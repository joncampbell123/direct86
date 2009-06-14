; counts RAM and provides INT 12h services

count_ram:      push    ax  ; stores counted lower RAM (in K) at addr 40:13 (just like standard legacy BIOS)
                push    cx
                push    dx
                push    ds
                xor     ax,ax
                mov     cx,ax
count_ram_1:    mov     ds,ax
                mov     dx,WORD [0000h]
                mov     WORD [0000h],55AAh
                cmp     WORD [0000h],55AAh
                mov     WORD [0000h],dx
                jne     count_ram_fin
                add     ax,(1024 >> 4)      ; increment 1KB
                inc     cx                  ; and count (in KB)
                cmp     ax,0A000h           ; but not too far...
                jb      count_ram_1
count_ram_fin:  mov     ax,40h
                mov     ds,ax
                mov     WORD [13h],cx       ; store conventional RAM count at 40:13
; install INT 12h handler
                xor     ax,ax
                mov     ds,ax
                mov     WORD [(12h << 2)],int_12_service
                mov     WORD [(12h << 2)+2],cs
                pop     ds
                pop     dx
                pop     cx
                pop     ax
                ret

; INT 12h service

int_12_service: cli
                push    ds
                push    si
                mov     si,40h
                mov     ds,si
                pop     si
                mov     ax,WORD [13h]
                pop     ds
                iret

