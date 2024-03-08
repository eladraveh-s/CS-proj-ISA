MAIN:
    add $a0, $zero, $zero, $zero, 0, 0 #init loop index i to 0
    LOOP1_IN_MAIN:
        add $a1, $zero, $zero, $zero, 0, 0 #init loop index j to 0
    LOOP2_IN_MAIN:
        jal $ra, $zero, $zero, $imm1, CREATE_CELL, 0 #function call CREATE_CELL
        add $a1, $a1, imm1, $zero, 1, 0 #j+=1; 
        bne $zero, $a1, $imm1, imm2, 4, LOOP2_IN_MAIN #if j != 4, go back to LOOP2_IN_MAIN
        add $a0, $a0, imm1, $zero, 1, 0 #i+=1;
        bne $zero, $a0, $imm1, imm2, 4, LOOP1_IN_MAIN #if i != 4, go back to LOOP1_IN_MAIN
    halt $zero, $zero, $zero, $zero, 0, 0 #halts execution
CREATE_CELL:
    #not really necessary allocates space in stack, cause im not touching the relevant registers
    add $t0, $zero, $zero, $zero, 0, 0 #init loop index k to 0
    add $t1, $zero, $zero, $zero, 0, 0 #init sum variable to 0
    LOOP_IN_CELL:
        mac $t2, $a0, $imm1, $t0, 4, 0 #t2 = i*4+k
        lw $t2, $imm1, $t2, $zero, 0x100 #t2 = matA[i][k]  (matA[i*4+k])
        mac $a2, $t0, $imm1, $a1, 4, 0 #a2 = k*4+j
        lw $a2, $imm1, $a2, $zero, 0x110 #a2 = matB[k][j] (matB[k*4+j])
        mac $t1, $a2, $t2, $t1, 0, 0 #v0 = matA[i][k]*matB[k][j]  + t1
        mac $t2, $a0, $imm1, $a1, 4, 0 #t2 = i*4+j                                                                                                                  
        add $t0, $t0, imm1, $zero, 1, 0 #k+=1; 
        bne $zero, $t0, $imm1, imm2, 4, LOOP_IN_CELL #if k != 4, go back to LOOP_IN_CELL
        sw  $t1, $t2, imm1, $zero, 0x120, 0 #matC[i][k] = t1 (the sum variable)
        jal $zero, $zero, $zero, $ra, 0, 0 #jump back to the $ra value 


