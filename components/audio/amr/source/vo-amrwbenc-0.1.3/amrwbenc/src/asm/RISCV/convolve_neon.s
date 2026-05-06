	.section  .text
        .global   Convolve_asm

Convolve_asm:
        addi sp, sp, -32
        sw ra, 28(sp)
        sw s0, 24(sp)
        sw s1, 20(sp)
        sw s2, 16(sp)
        sw s3, 12(sp)
        sw s4, 8(sp)
        sw s5, 4(sp)
        sw s6, 0(sp)

        li s0, 0
        li s6, 0x8000

LOOP:
        li s1, 0
        add s2, s0, s1
        add s3, s0, 1
        mv s4, a0
        lh s5, 0(s4)
        add s4, s1, s3, s3
        lh s7, -2(s4)
        mul s8, s5, s7

LOOP1:
        bge s3, zero, L1
        add s4, s4, -8
        mv s5, s4
        vle16.v v0, (s4)
        vle16.v v1, (s5)
        vrev64.v v1, v1
        addi s3, s3, -4
        vwmacc.vv v10, v0, v1
        j LOOP1
L1:
        vadd.vv v20, v20, v21
        vpadd.vv v20, v20, v20
        vmv.s.x s5, v20[0]
        add s5, s5, s8
        add s5, s6, s5, s5
        srl s5, s5, 16
        addi s0, s0, 1
        mv s4, a2
        sh s5, 0(s4)

        li s1, 0
        add s2, s0, s1
        add s3, s1, 1
        mv s4, a0
        lh s5, 0(s4)
        lh s7, -2(s2)
        lh s8, 2(s4)
        lh s9, 0(s2)
        mul s10, s5, s7
        madd s10, s8, s9, s10

        vsetvli t0, s3, e16, m1
        li t1, 0
        add s4, s2, t0, sll
LOOP2:
        bge s3, zero, L2
        add s4, s4, -8
        mv s5, s4
        vle16.v v0, (s4)
        vle16.v v1, (s5)
        addi s3, s3, -1
        vwmacc.vv v10, v0, v1
        j LOOP2
L2:
        vadd.vv v20, v20, v21
        vpadd.vv v20, v20, v20
        vmv.s.x s5, v20[0]
        add s10, s10, s5
        add s10, s6, s10, s10
        srl s10, s10, 16
        addi s0, s0, 1
        mv s4, a2
        sh s10, 0(s4)

        li s1, 0
        add s2, s0, s1
        add s3, s1, 1
        mv s4, a0
        lh s5, 0(s4)
        lh s7, -2(s2)
        lh s8, 2(s4)
        lh s9, 0(s2)
        mul s10, s5, s7
        madd s10, s8, s9, s10
        lh s5, 2(s4)
        lh s7, -2(s2)
        lh s8, 4(s4)
        lh s9, 0(s2)
        madd s10, s5, s7, s10
        madd s10, s8, s9, s10

        vsetvli t0, s3, e16, m1
        li t1, 0
        add s4, s2, t0, sll
LOOP3:
        bge s3, zero, L3
        add s4, s4, -8
        mv s5, s4
        vle16.v v0, (s4)
        vle16.v v1, (s5)
        addi s3, s3, -1
        vwmacc.vv v10, v0, v1
        j LOOP3
L3:
        vadd.vv v20, v20, v21
        vpadd.vv v20, v20, v20
        vmv.s.x s5, v20[0]
        add s10, s10, s5
        add s10, s6, s10, s10
        srl s10, s10, 16
        addi s0, s0, 1
        mv s4, a2
        sh s10, 0(s4)

        add s3, s1, 1
        add s4, s1, s3, s3
        mv s5
