.define sz = rg
LOOP: jmp L1
prrrn #-5
mov STR[LOOP], STR[2]
red #4
lea r2 ,#-4
cmp r3, #4
bne r2

LIST: .data 6, -9, len
K: .data LIST
