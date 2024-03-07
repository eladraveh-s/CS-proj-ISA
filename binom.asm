add $s0, $zero, $zero, $zero, 0, 0 # Setting up initial values
lw $a0, $imm1, $zero, $zero, 256, 0
lw $a1, $imm1, $zero, $zero, 257, 0
jal $ra, $zero, $zero, $imm1, Binom, 0
sw $imm1, $zero, $s0, $zero, 258, 0
halt $zero, $zero, $zero, $zero, 0, 0

Binom:
    beq $zero, $a0, $a1, $imm1, OneBin, 0
    beq $zero, $a1, $zero, $imm1, OneBin, 0
    jal $t0, $zero, $zero, $imm1, GenBin, 0

OneBin:
    add $s0, $s0, $imm1, $zero, 1, 0
    jal $t0, $zero, $zero, $ra, 0, 0

GenBin:
    sub $sp, $sp, $imm1, $zero, 3, 0 # Storing parameters
    sw $sp, $imm1, $a0, $zero, 1, 0
    sw $sp, $imm1, $a1, $zero, 2, 0
    sw $sp, $imm1, $ra, $zero, 3, 0
    sub $a0, $a0, $imm1, $zero, 1, 0 # Updating arguments
    sub $a1, $a1, $imm1, $zero, 1, 0
    jal $ra, $zero, $zero, $imm1, Binom, 0
    lw $a0, $imm1, $sp, $zero, 1, 0 # Restoring parameters
    lw $a1, $imm1, $sp, $zero, 2, 0
    lw $ra, $imm1, $sp, $zero, 3, 0
    sub $a0, $a0, $imm1, $zero, 1, 0 # Updating arguments
    jal $ra, $zero, $zero, $imm1, Binom, 0
    lw $a0, $imm1, $sp, $zero, 1, 0 # Restoring parameters
    lw $a1, $imm1, $sp, $zero, 2, 0
    lw $ra, $imm1, $sp, $zero, 3, 0
    add $sp, $sp, $imm1, $zero, 3, 0
    jal $t0, $zero, $zero, $ra, 0, 0
