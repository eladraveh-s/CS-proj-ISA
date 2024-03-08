MAIN:
    add $a0, $zero, $imm1, $zero, 7, 0 #init loop index i to 7
    out $zero, $imm1, $zero, imm2, 16, 0 #diskbuffer = 0. its always zero, cause im doing it one by one.
    LOOP:
        jal $ra, $zero, $zero, $imm1, MOVE_SECTOR, 0 #function call MOVE_SECTOR
        sub $a0, $a0, imm1, $zero, 1, 0 #i-=1;
        bne $zero, $a0, $imm1, imm2, 0, LOOP #if i != 0, go back to LOOP
    halt $zero, $zero, $zero, $zero, 0, 0 #halts execution
MOVE_SECTOR:
    WHILE1:
        in $t0, $imm1, $zero, $zero, 17, 0 #t0 = diskstatus
        bne $zero, $t0, $imm1, imm2, 0, WHILE1 #if t0 != 0, go back to WHILE1
    out $zero, $imm1, $zero, $a0, 15, 0 #disksector = i
    out $zero, $imm1, $zero, $imm2, 14, 1 #diskcmd = 1 (read)
    WHILE2:
        in $t0, $imm1, $zero, $zero, 17, 0 #t0 = diskstatus
        bne $zero, $t0, $imm1, imm2, 0, WHILE1 #if t0 != 0, go back to WHILE2
    add $t1, $a0, imm1, $zero, 1, 0 # t1 = i+1; 
    out $zero, $imm1, $zero, $a1, 15, 0 #disksector = i+1
    out $zero, $imm1, $zero, $imm2, 14, 2 #diskcmd = 2 (write)
    jal $zero, $zero, $zero, $ra, 0, 0 #jump back to the $ra value
