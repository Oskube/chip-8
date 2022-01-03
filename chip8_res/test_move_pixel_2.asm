; Go through all pixels on display starting from top-left
; and ending to bottom-right and then start over
; Set pixel
; Wait
; Disable pixel
; MOV VF 0
; Shift mask to right
; If Vf == 1
;   Increase x_byte
;   If x_byte > Width/8
;      Inc y
;      If y > Height
;        y = 0
; JMP to start

; Set initial values
SPRITE = 0x223
MAXX   = 64
MAXY   = 32

MOV V2, 5
MOV I, SPRITE

ResetY: MOV V1, 0 ; y
ResetX: MOV V0, 0 ; x
Clear:  CLS
        DRW V0, V1, 1
        ADD V0, 1 ; x++
        MOV DT, V2

TimerLoop:
    MOV V3, DT
SE V3, 0
  JMP TimerLoop

; Check boundaries
SE  V0, MAXX    ; if x not over max width
  JMP Clear     ; jump back to clear
ADD  V1, 1      ; y++
SNE  V1, MAXY   ; if y over max height
    JMP ResetY  ; jump back to start and set everything 0
JMP ResetX
NOP 128  ; SPRITE
