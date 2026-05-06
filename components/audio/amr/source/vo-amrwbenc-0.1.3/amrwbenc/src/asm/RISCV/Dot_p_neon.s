Dot_product12_asm:
    addi sp, sp, -16
    sw ra, 0(sp)
    sw s0, 4(sp)
    sw s1, 8(sp)
    sw s2, 12(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2
    li t0, 0
    beq s0, s1, LOOP_EQ

    LOOP:
        lw t1, 0(s0)
        lw t2, 0(s1)
        mul t3, t1, t2
        addi s0, s0, 4
        addi s1, s1, 4
        add t0, t0, t3
        addi s2, s2, -1
        bnez s2, LOOP

    LOOP_EQ:
        li s2, 12
        li t0, 0
        LOOP_EQ_1:
            lw t1, 0(s0)
            lw t2, 0(s1)
            mul t3, t1, t2
            addi s0, s0, 4
            addi s1, s1, 4
            add t0, t0, t3
            addi s2, s2, -1
            bnez s2, LOOP_EQ_1

    li t1, 0
    li t2, 0
    li t3, 0
    li t4, 0
    li t5, 0
    li t6, 0
    li t7, 0
    li t8, 0
    li t9, 0
    li t10, 0
    li t11, 0
    li t12, 0
    li t13, 0
    li t14, 0
    li t15, 0
    li t16, 0
    li t17, 0
    li t18, 0
    li t19, 0
    li t20, 0
    li t21, 0
    li t22, 0
    li t23, 0
    li t24, 0
    li t25, 0
    li t26, 0
    li t27, 0
    li t28, 0
    li t29, 0
    li t30, 0
    li t31, 0

    li s2, 64
    beq s2, zero, Lable1

    Lable1:
        add t0, t0, t1
        add t0, t0, t2
        add t0, t0, t3
        add t0, t0, t4
        add t0, t0, t5
        add t0, t0, t6
        add t0, t0, t7
        add t0, t0, t8
        add t0, t0, t9
        add t0, t0, t10
        add t0, t0, t11
        add t0, t0, t12
        add t0, t0, t13
        add t0, t0, t14
        add t0, t0, t15
        add t0, t0, t16
        add t0, t0, t17
        add t0, t0, t18
        add t0, t0, t19
        add t0, t0, t20
        add t0, t0, t21
        add t0, t0, t22
        add t0, t0, t23
        add t0, t0, t24
        add t0, t0, t25
        add t0, t0, t26
        add t0, t0, t27
        add t0, t0, t28
        add t0, t0, t29
        add t0, t0, t30
        add t0, t0, t31

    addi sp, sp, 16
    lw ra, 0(sp)
    lw s0, 4(sp)
    lw s1, 8(sp)
    lw s2, 12(sp)
    jr ra


