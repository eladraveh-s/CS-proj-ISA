lw $s0, $imm1, $zero, $zero, 256, 0            # $s0 = radius
mac $s0, $s0, $s0, $zero, 0, 0                 # $s0 = radius squared
add $s2, $zero, $zero, $zero, 0, 0             # $s2 = pixel y cor

OutLoop:
    add $s1, $zero, $zero, $zero, 0, 0         # $s1 = pixel x cor
    sub $t1, $s2, $imm1, $zero, 128, 0         # $t1 = y distance from the middle (may be negative)
    mac $t1, $t1, $t1, $zero, 0, 0             # $t1 = y dist squared

InLoop:
    sub $t0, $s1, $imm1, $zero, 128, 0         # $t0 = x distance from the middle (may be negative)
    mac $t0, $t0, $t0, $zero, 0, 0             # $t0 = x dist squared
    add $t0, $t0, $t1, $zero, 0, 0             # $t0 = x dist squared + y dist squared
    ble $zero, $t0, $s0, $imm1, ColorWhite, 0  # If inside the circle go to color white
    jal $t2, $zero, $zero, $imm1, FinInLoop, 0 # If outside the circle got to FinInLoop

ColorWhite:
    mac $t2, $s2, $imm1, $s1, 256, 0           # pixel number in the array
    out $zero, $imm1, $zero, $t2, 20, 0        # load pixel num to IO register
    out $zero, $imm1, $zero, $imm2, 21, 255    # Color pixel white
    out $zero, $imm1, $zero, $imm2, 22, 1      # Change pixel
    out $zero, $imm1, $zero, $imm2, 22, 0      # Stop changing pixels

FinInLoop:
    add $s1, $s1, $imm1, $zero, 1, 0           # Advance inner loop
    blt $zero, $s1, $imm1, $imm2, 256, InLoop 
    add $s2, $s2, $imm1, $zero, 1, 0           # Advance outer loop
    blt $zero, $s2, $imm1, $imm2, 256, OutLoop

halt $zero, $zero, $zero, $zero, 0, 0

.word 0x100 50
