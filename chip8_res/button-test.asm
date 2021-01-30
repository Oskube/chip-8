; Read keypress and display it
MOV V0, K
MOV F, V0
CLS
DRW  V1, V1, 5
; Program beginning is 0x200 (512)
JMP 512
