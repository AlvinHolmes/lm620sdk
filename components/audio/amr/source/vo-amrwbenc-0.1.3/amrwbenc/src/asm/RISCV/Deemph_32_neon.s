.global Deemph_32_asm
Deemph_32_asm:
    addi sp, sp, -16
    sw ra, 0(sp)
    sw s0, 4(sp)
    sw s1, 8(sp)
    sw s2, 12(sp)

    li s0, 2
    lw s1, 0(a0)
    lw s2, 0(a1)
    li s3, 22282
    li s4, 0x8000
    lw s5, 0(a5)

    #y[0]
    slli s6, s1, 16
    srai s7, s2, 4
    slli s7, s7, 4
    add s8, s6, s7
    slli s8, s8, 3
    mul s9, s5, s4
    add s8, s8, s9
    addi a0, a0, 4
    addi a1, a1, 4
    qadd s8, s8, s4
    srai s8, s8, 16
    sw s8, 0(a2)

    #y[1]
    slli s6, s1, 16
    srai s7, s2, 4
    slli s7, s7, 4
    add s8, s6, s7
    slli s8, s8, 3
    mul s9, s8, s4
    addi a0, a0, 4
    addi a1, a1, 4
    addi a2, a2, 2
    addi s0, s0, 2
    qadd s8, s8, s4
    srai s8, s8, 16
    sw s8, 0(a2)

LOOP:
    lw s1, 0(a0)
    lw s2, 0(a1)
    slli s6, s1, 16
    srai s7, s2, 4
    slli s7, s7, 4
    add s8, s6, s7
    slli s8, s8, 3
    mul s9, s8, s4
    addi a0, a0, 4
    addi a1, a1, 4
    addi a2, a2, 2
    addi s0, s0, 2
    qadd s8, s8, s4
    srai s8, s8, 16
    sw s8, 0(a2)

    lw s1, 0(a0)
    lw s2, 0(a1)
    slli s6, s1, 16
    srai s7, s2, 4
    slli s7, s7, 4
    add s8, s6, s7
    slli s8, s8, 3
    mul s9, s8, s4
    addi a0, a0, 4
    addi a1, a1, 4
    addi a2, a2, 2
    addi s0, s0, 2
    qadd s8, s8, s4
    srai s8, s8, 16
    sw s8, 0(a2)

    bne s0, 64, LOOP

    sw s8, 0(a5)
    sw s8, 0(a2)

    lw ra, 0(sp)
    lw s0, 4(sp)
    lw s1, 8(sp)
    lw s2, 12(sp)
    addi sp, sp, 16

    ret


