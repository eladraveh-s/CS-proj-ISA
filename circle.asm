lw $s0, $imm1, $zero, $zero, 256, 0 # $s0 = radius
mac $s0, $s0, $s0, $zero, 0, 0 # $s0 = radius squared
add $s2, $zero, $zero, $zero, 0, 0 # $s2 = pixel y cor

OutLoop:
    add $s1, $zero, $zero, $zero, 0, 0 # $s1 = pixel x cor

InLoop:
    mac $t0, $s1, $s1, $zero, 0, 0 # $t0 = x cor squared
    mac $t0, $s2, $s2, $t0, 0, 0 # t0 = x cor squared + y cor squared
    mac $t1, $s1, $imm1, $s2, 256, 0 # pixel number in the array
    out $zero, $imm1, $zero, $t1, 20, 0 # load pixel num to IO register
    ble $zero, $t0, $s0, $ColorWhite # If inside the circle fo to color white
    jal $t2, 0, 0, $imm1, ColorBlack, 0 # If outside the circle fo to color black

ColorWhite:
    out $zero, $imm1, $zero, $imm2, 21, 255
    
ColorBlack:
    out $zero, $imm1, $zero, $imm2, 21, 0

FinInLoop:
    out $zero, $imm1, $zero, $imm2, 22, 1
    out $zero, $imm1, $zero, $imm2, 22, 0
    blt $zero, $s1, $imm1, $imm2, 256, InLoop
    add $s2, $imm1, $zero, $zero, 1, 0
    blt $zero, $s2, $imm1, $imm2, 256, OutLoop