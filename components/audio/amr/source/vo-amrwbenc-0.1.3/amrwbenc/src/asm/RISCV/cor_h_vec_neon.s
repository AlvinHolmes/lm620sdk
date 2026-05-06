.section .text
.global cor_h_vec_012_asm

cor_h_vec_012_asm:
    addi sp, sp, -32
    sw ra, 28(sp)
    sw s0, 24(sp)
    sw s1, 20(sp)
    sw s2, 16(sp)
    sw s3, 12(sp)
    sw s4, 8(sp)
    sw s5, 4(sp)
    sw s6, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2
    mv s3, a3
    mv s4, a4
    mv s5, a5
    mv s6, a6

    li t0, 0
    li t1, 0
    li t2, 0
    li t3, 0
    li t4, 0
    li t5, 0
    li t6, 0

LOOPi:
    li t5, 0
    li t6, 0
    slli t7, s2, 5
    add t7, t7, s4
    add t8, t7, t0, sll=1
    add t9, t0, zero
    li t10, 62
    sub t11, t10, s2

LOOPj1:
    lh t12, 0(s0)
    lh t13, 0(t8)
    lh t14, 2(t8)
    sub t11, t11, 1
    mul t5, t12, t13
    mul t6, t12, t14
    bge t11, zero, LOOPj1

    lh t12, 0(s0)
    slli t6, t6, 2
    mul t5, t12, t14
    li t14, 0x8000
    slli t5, t5, 2
    add t6, t6, t14
    add t5, t5, t14
    srli t5, t5, 16
    srli t6, t6, 16
    add t7, s3, s2, sll=1
    add t8, t7, 32
    lh t9, 0(t7)
    lh t10, 2(t7)
    mul t12, t5, t9
    mul t14, t6, t10
    srai t5, t12, 15
    srai t6, t14, 15
    lw t7, 40(sp)
    lw t8, 44(sp)
    lh t9, 0(t7)
    lh t10, 0(t8)
    add t9, t9, t0, sll=1
    add t10, t10, t0, sll=1
    add t5, t5, t12
    add t6, t6, t14
    sh t5, 0(t9)
    sh t6, 0(t10)

    addi t0, t0, 4

    li t5, 0
    li t6, 0
    add t8, t7, 32
    add t9, t1, t0, sll=1
    add t10, t0, zero
    sub t11, t10, 62

LOOPj2:
    lh t12, 0(s0)
    lh t13, 0(t9)
    lh t14, 2(t9)
    sub t11, t11, 1
    mul t5, t12, t13
    mul t6, t12, t14
    bge t11, zero, LOOPj2

    lh t12, 0(s0)
    slli t6, t6, 2
    mul t5, t12, t14
    li t14, 0x8000
    slli t5, t5, 2
    add t6, t6, t14
    add t5, t5, t14
    srli t5, t5, 16
    srli t6, t6, 16
    add t7, s3, s2, sll=1
    add t8, t7, 32
    lh t9, 0(t7)
    lh t10, 2(t7)
    mul t12, t5, t9
    mul t14, t6, t10
    srai t5, t12, 15
    srai t6, t14, 15
    lw t7, 40(sp)
    lw t8, 44(sp)
    lh t9, 0(t7)
    lh t10, 0(t8)
    add t9, t9, t0, sll=1
    add t10, t10, t0, sll=1
    add t5, t5, t12
    add t6, t6, t14
    sh t5, 0(t9)
    sh t6, 0(t10)

    addi t0, t0, 4
    addi t4, t4, 1
    blt t4, 3, LOOPi

    lw ra, 28(sp)
    lw s0, 24(sp
