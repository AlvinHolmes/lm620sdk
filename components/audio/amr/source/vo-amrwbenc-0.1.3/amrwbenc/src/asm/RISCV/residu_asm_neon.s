.section .text
.globl Residu

Residu:
    addi sp, sp, -16
    sw ra, 0(sp)
    sw s0, 4(sp)

    mv s0, a0
    mv a0, a1
    mv a1, a2
    mv a2, a3

    li t0, 4
    sub t1, a3, t0

    vsetvli t0, t0, e16, m1
    vle16.v v0, (s0)
    addi s0, s0, 16

    vsetvli t0, t0, e32, m1
    vfmv.v.f v1, v0
    vfmv.v.f v2, v0
    vfmv.v.f v3, v0
    vfmv.v.f v4, v0
    vfmv.v.f v5, v0
    vfmv.v.f v6, v0
    vfmv.v.f v7, v0
    vfmv.v.f v8, v0

LOOP1:
    add t2, a0, t1
    add t3, a1, t1
    mv t4, t2

    vsetvli t0, t0, e16, m4
    vle16.v v9, (t4)
    addi t4, t4, 8
    vle16.v v10, (t4)
    addi t4, t4, 8
    vle16.v v11, (t4)
    addi t4, t4, 8
    vle16.v v12, (t4)
    addi t4, t4, 8

    vsetvli t0, t0, e32, m1
    vwmul.vv v13, v9, v0
    vwmacc.vv v13, v10, v1, v13
    vwmacc.vv v13, v11, v2, v13
    vwmacc.vv v13, v12, v3, v13
    vwmacc.vv v13, v0, v4, v13
    vwmacc.vv v13, v0, v5, v13
    vwmacc.vv v13, v0, v6, v13
    vwmacc.vv v13, v0, v7, v13
    vwmacc.vv v13, v0, v8, v13

    sub t4, t2, 16
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v1, v0
    sub t4, t2, 32
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v2, v0
    sub t4, t2, 48
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v3, v0
    sub t4, t2, 64
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v4, v0
    sub t4, t2, 80
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v5, v0
    sub t4, t2, 96
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v6, v0
    sub t4, t2, 112
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v7, v0
    sub t4, t2, 128
    vsetvli t0, t0, e16, m1
    vle16.v v9, (t4)
    vsetvli t0, t0, e32, m1
    vwmacc.vv v13, v9, v8, v0

    sub t1, t1, 16
    vsetvli t0, t0, e32, m1
    vadd
