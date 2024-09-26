# Branching example
movl r1 0 5        # Load 5 into register 1
cmp r1 0           # Compare r1 with 0
je done            # If r1 == 0, jump to done
add r1 r1 r2       # Decrement r1 (not taken)
done: halt         # Halt
