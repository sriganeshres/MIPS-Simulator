.data
    num1: .word 25
    num2: .word 47
    num3: .word 35
    max_value: .word 0                # To store the maximum value

.text
    lw $t0, num1                      # Load num1 into $t0
    lw $t1, num2                      # Load num2 into $t1
    lw $t2, num3                      # Load num3 into $t2
    slt $t3, $t0, $t1                 # Set $t3 to 1 if $t0 < $t1, else 0
    beq $t3, $zero, check_num3         # If num1 >= num2, jump to check_num3
    add $t4, $zero, $t1               # Else, max = num2
    j check_num3

check_num3:
    slt $t3, $t4, $t2                 # Set $t3 to 1 if current max < num3
    beq $t3, $zero, end               # If max >= num3, jump to end
    add $t4, $zero, $t2               # Else, max = num3

end:
    # Store the result in max_value
    lw $t5, max_value                 # Load max_value to $t5
    add $t5, $zero, $t4               # Move the max into $t5 (result)
