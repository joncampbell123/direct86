; this is the source for the emulator's ROM.
; It must fit within 64K and not cover the areas that have special meaning:
;
; 0x0100 - 0x0120               Diskette Parameter Table (Drive A:)
; these areas must be filled with IRETs
; 0xAB00 - 0xABFF               Simulator patches for BIOS services (depending
;                               on the address the executioneer does the work)
; 0xCC00 - 0xCCFF               Simulator patches for IRQ's
; 0xEC00 - 0xECFF               Simulator patches for exceptions

; During "RESET", the executioneer starts at F000:FFF0 like a normal PC.
; for other signals, execution starts in F000:0000 with signal codes in
; AX, BX, CX, and DX. Pointers in SI, DI, DS, SS, and ES.

; make sure any untouched part has IRET in it
;--------------------------------------------

                org     0

                times   0100h-($-$$) DB 0CFh

config_prog:
incbin "config/config.com"

                times   0C000h-($-$$) DB 0CFh
; code begins here
; ----------------

%include "boot/int19.inc"
%include "boot/noboot.inc"              ; routine for no-boot error message
%include "boot/reset.inc"               ; routine for reset

; RAM routines
%include "mem/countram.inc"

; video BIOS routines
%include "video/text.inc"

; keyboard BIOS routines
%include "keyboard/keyboard.inc"

; CMOS/RTC routines
%include "cmos/cmos.inc"

; Timer routines
%include "timer/setup.inc"

; codeholio's signature
;----------------------

                times   0FF60h-($-$$) DB 0CFh

                db      'ROM BIOS v1.00 (C) 1998-2001 Jonathan Campbell'

; RESET ROUTINE (just like a normal PC)
; -------------------------------------
                times   0FFF0h-($-$$) DB 0CFh
                jmp     reset

; PAD TO 64K (the full size possible of a ROM BIOS)
; -------------------------------------
                times   010000h-($-$$) DB 0CFh
