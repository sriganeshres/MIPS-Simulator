.data
    counter: .word 0
.text
    lw $t0, counter     # Load counter into $t0
    li $t1, 5           # Set a limit of 5
loop:
    addi $t0, $t0, 1    # Increment counter
    beq $t0, $t1, end   # If counter == 5, jump to 'end'
    j loop              # Otherwise, repeat loop
end:
    addi $t2, $t2, 16     
