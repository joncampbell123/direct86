
; IRQ 0 - init

irq0_init:              cli
                        xor     ax,ax
                        mov     ds,ax
; setup IRQ 0 handler
                        mov     di,8 << 2
                        mov     [di],WORD IRQ0
                        mov     [di+2],WORD 0F000h
; and link up BIOS timer tick interrupt 1Ch
                        mov     di,1Ch << 2
                        mov     [di],WORD TimerTickDefault_1C
                        mov     [di+2],WORD 0F000h
                        sti
                        ret

; IRQ0 handler

IRQ0:                   push    ax
                        push    bx
                        push    cx
                        push    dx
                        int     1Ch
                        mov     al,20h
                        out     20h,al
	                    mov		al,60h
		                out		20h,al
                        pop     dx
                        pop     cx
                        pop     bx
                        pop     ax
                        iret

; INT 1Ch timer tick

TimerTickDefault_1C:    iret

