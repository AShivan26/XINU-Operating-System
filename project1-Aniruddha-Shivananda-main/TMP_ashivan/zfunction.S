.globl zfunction

zfunction:
    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %eax
    andl $0xFFFFF00F, %eax 
    shll $8, %eax
    orl $0x000000FF, %eax

    movl %ebp, %esp
    popl %ebp
    ret