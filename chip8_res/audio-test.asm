MOV V0, 0
MOV V1, 30 ; Delay
MOV DT, V0
MOV ST, V0
CALL 530
MOV DT, V1
ADD V0, 10
CALL 530
JMP 516
; Subroutine to wait DT to 0
MOV V2, DT
SE V2, 0
JMP 530
RET
