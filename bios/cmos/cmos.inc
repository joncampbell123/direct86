; CMOS code

getcmosdata:            out         70h,al
                        jmp         short $+2
                        in          al,71h
                        ret

setcmosdata:            push        ax
                        mov         al,ah
                        out         70h,al
                        pop         ax
                        out         71h,al
                        ret

