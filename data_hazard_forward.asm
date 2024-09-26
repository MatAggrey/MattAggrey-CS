movl r1 0 5        # Load 5 into register r1
movl r2 0 10       # Load 10 into register r2
add r3 r1 r2       # r3 = r1 + r2
nand r4 r3 r2      # r4 = ~(r3 & r2) (depends on r3)
halt
