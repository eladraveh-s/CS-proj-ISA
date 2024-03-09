add $s0, $zero, $zero, $zero, 0, 0          # Setting up initial values
lw $a0, $imm1, $zero, $zero, 256, 0
lw $a1, $imm1, $zero, $zero, 257, 0
jal $ra, $zero, $zero, $imm1, Binom, 0      # Starts function
sw $s0, $imm1, $zero, $zero, 258, 0
halt $zero, $zero, $zero, $zero, 0, 0

Binom:
    beq $zero, $a0, $a1, $imm1, OneBin, 0   # Checks for condition 1
    beq $zero, $a1, $zero, $imm1, OneBin, 0 # Checks for condition 2
    jal $t0, $zero, $zero, $imm1, GenBin, 0 # Jumps to general case

OneBin:
    add $s0, $s0, $imm1, $zero, 1, 0        # Handling of basic cases
    jal $t0, $zero, $zero, $ra, 0, 0

GenBin:
    sub $sp, $sp, $imm1, $zero, 3, 0        # Storing parameters in stack
    sw $a0, $sp, $imm1, $zero, 0, 0
    sw $a1, $sp, $imm1, $zero, 1, 0
    sw $ra, $sp, $imm1, $zero, 2, 0
    sub $a0, $a0, $imm1, $zero, 1, 0        # Updating arguments
    sub $a1, $a1, $imm1, $zero, 1, 0
    jal $ra, $zero, $zero, $imm1, Binom, 0  # Entering recursive call with n - 1. k - 1
    lw $a0, $imm1, $sp, $zero, 0, 0         # Restoring parameters
    lw $a1, $imm1, $sp, $zero, 1, 0
    lw $ra, $imm1, $sp, $zero, 2, 0
    sub $a0, $a0, $imm1, $zero, 1, 0        # Updating arguments
    jal $ra, $zero, $zero, $imm1, Binom, 0  # Entering recursive call with n - 1. k
    lw $a0, $imm1, $sp, $zero, 0, 0         # Restoring parameters from stack
    lw $a1, $imm1, $sp, $zero, 1, 0
    lw $ra, $imm1, $sp, $zero, 2, 0
    add $sp, $sp, $imm1, $zero, 3, 0
    jal $t0, $zero, $zero, $ra, 0, 0        # Exits current Binom call
