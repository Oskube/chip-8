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
MOV V2, 5
MOV I, 546 ; 0x201. sprite used
MOV V1, 0 ; y
MOV V0, 0 ; x;  p: 0x206 518
CLS ; 0x208
DRW V0, V1, 1
ADD V0, 1 ; x++
MOV DT, V2
MOV V3, DT
SE V3, 0
  JMP 528
SE V0, 64 ; if x not over max width
  JMP 520  ; jump back to 0x208
ADD V1, 1  ; y++
SNE  V1, 32 ; if y over max height
    JMP 516; 0x204 jump back to start and set everything 0
JMP 518 ; 0x206
NOP 0x8000 ; compiles to MOV V0, V0 but shouldn't be run
