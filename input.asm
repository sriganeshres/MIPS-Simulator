.data  
var1: .word 5  
var2: .word 10  

.text  
.globl main

main:  
    lw $t0, var1    # Load var1 into $t0  
    lw $t1, 0($t0)    # Load var2 into $t1  
    add $t2, $t0, $t1 # Add $t0 and $t1, store result in $t2  
    sub $t3, $t1, $t0 # Subtract $t0 from $t1, store result in $t3  
    beq $t2, $t3, hello # Branch to 'end' if $t2 equals $t3  
    j main          # Jump back to the main label  

hello:
    j main