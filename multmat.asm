MAIN:
    add $a0, $zero, $zero, $zero, 0, 0 #init loop index i to 0
LOOP1_IN_MAIN:
    add $a1, $zero, $zero, $zero, 0, 0 #init loop index j to 0
LOOP2_IN_MAIN:
    jal $ra, $zero, $zero, $imm1, CREATE_CELL, 0 #function call CREATE_CELL
    add $a1, $a1, $imm1, $zero, 1, 0 #j+=1; 
    bne $zero, $a1, $imm1, $imm2, 4, LOOP2_IN_MAIN #if j != 4, go back to LOOP2_IN_MAIN
    add $a0, $a0, $imm1, $zero, 1, 0 #i+=1;
    bne $zero, $a0, $imm1, $imm2, 4, LOOP1_IN_MAIN #if i != 4, go back to LOOP1_IN_MAIN
halt $zero, $zero, $zero, $zero, 0, 0 #halts execution
CREATE_CELL:
    add $t0, $zero, $zero, $zero, 0, 0 #init loop index k to 0
    add $t1, $zero, $zero, $zero, 0, 0 #init sum variable to 0
LOOP_IN_CELL:
    mac $t2, $a0, $imm1, $t0, 4, 0 #t2 = i*4+k
    lw $t2, $imm1, $t2, $zero, 0x100, 0 #t2 = matA[i][k]  (matA[i*4+k])
    mac $a2, $t0, $imm1, $a1, 4, 0 #a2 = k*4+j
    lw $a2, $imm1, $a2, $zero, 0x110, 0 #a2 = matB[k][j] (matB[k*4+j])
    mac $t1, $a2, $t2, $t1, 0, 0 #v0 = matA[i][k]*matB[k][j]  + t1
    mac $t2, $a0, $imm1, $a1, 4, 0 #t2 = i*4+j                                                                                                                  
    add $t0, $t0, $imm1, $zero, 1, 0 #k+=1; 
    bne $zero, $t0, $imm1, $imm2, 4, LOOP_IN_CELL #if k != 4, go back to LOOP_IN_CELL
    sw  $t1, $t2, $imm1, $zero, 0x120, 0 #matC[i][k] = t1 (the sum variable)
    jal $zero, $zero, $zero, $ra, 0, 0 #jump back to the $ra value 
.word 0x100 1
.word 0x101 1
.word 0x102 1
.word 0x103 1
.word 0x104 1
.word 0x105 1
.word 0x106 1
.word 0x107 1
.word 0x108 1
.word 0x109 1
.word 0x10A 1
.word 0x10B 1
.word 0x10C 1
.word 0x10D 1
.word 0x10E 1
.word 0x10F 1
.word 0x110 1
.word 0x111 1
.word 0x112 1
.word 0x113 1
.word 0x114 1
.word 0x115 1
.word 0x116 1
.word 0x117 1
.word 0x118 1
.word 0x119 1
.word 0x11A 1
.word 0x11B 1
.word 0x11C 1
.word 0x11D 1
.word 0x11E 1
.word 0x11F 1
