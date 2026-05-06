.section  .text
        .global    Norm_corr_asm
        .extern    Convolve_asm
        .extern    Isqrt_n
@******************************
@ constant
@******************************
.equ    EXC               , 0
.equ    XN                , 4
.equ    H                 , 8
.equ    L_SUBFR           , 12
.equ    voSTACK           , 172
.equ    T_MIN             , 212
.equ    T_MAX             , 216
.equ    CORR_NORM         , 220

Norm_corr_asm:

        addi          sp, sp, -voSTACK
        sw            ra, 168(sp)
        sw            s0, 164(sp)
        sw            s1, 160(sp)
        sw            s2, 156(sp)
        sw            s3, 152(sp)
        sw            s4, 148(sp)
        sw            s5, 144(sp)
        sw            s6, 140(sp)
        sw            s7, 136(sp)
        sw            s8, 132(sp)
        sw            s9, 128(sp)
        sw            s10, 124(sp)
        sw            s11, 120(sp)

        addi          s2, zero, 20                 @get the excf[L_SUBFR]
        lw            s4, T_MIN(sp)            @get t_min
        addi          s5, s0, -s4*2          @get the &exc[k]

        @transfer Convolve function
        addi          sp, sp, -16
        sw            s0, 0(sp)
        sw            s1, 4(sp)
        sw            s2, 8(sp)
        addi          a0, s5, 0
        addi          a1, s2, 0
        jal           Convolve_asm
        lw            s0, 0(sp)
        lw            s1, 4(sp)
        lw            s2, 8(sp)
        addi          sp, sp, 16

        @ s2 --- excf[]

        mv            s7, 1
        vsetvli       t0, s0, e16, m1
        vle16.v       v0, (s1)
        addi          s1, s1, 16
        vle16.v       v1, (s1)
        addi          s1, s1, 16
        vle16.v       v2, (s1)
        addi          s1, s1, 16
        vle16.v       v3, (s1)
        addi          s1, s1, 16

        vwmul.vx      v4, v0, v0
        vwmul.vx      v5, v1, v1
        vwmul.vx      v6, v2, v2
        vwmul.vx      v7, v3, v3
        vwmul.vx      v8, v0, v0
        vwmul.vx      v9, v1, v1
        vwmul.vx      v10, v2, v2
        vwmul.vx      v11, v3, v3
        vwmul.vx      v12, v0, v0
        vwmul.vx      v13, v1, v1
        vwmul.vx      v14, v2, v2
        vwmul.vx      v15, v3, v3
        vwmul.vx      v16, v0, v0
        vwmul.vx      v17, v1, v1
        vwmul.vx      v18, v2, v2
        vwmul.vx      v19, v3, v3

        vadd.vx       v20, v4, v5
        vadd.vx       v21, v6, v7
        vadd.vx       v22, v8, v9
        vadd.vx       v23, v10, v11
        vadd.vx       v24, v12, v13
        vadd.vx       v25, v14, v15
        vadd.vx       v26, v16, v17
        vadd.vx       v27, v18, v19

        vadd.vx       v28, v20, v21
        vadd.vx       v29, v22, v23
        vadd.vx       v30, v24, v25
        vadd.vx       v31, v26, v27

        vadd.vx       v32, v28, v29
        vadd.vx       v33, v30, v31

        vadd.vx       v34, v32, v33

        vadd.vx       v35, v34, s7
        vsetvli       t0, s0, e32, m1
        vredsum.vs    v36, v35
        vmv.x.s32     s9, v36

        slli          s10, s9, 1
        addi          s10, s10, 1
        clz           s7, s10
        sub           s6, zero, s7
        addi          s7, s6, 32
        srai          s6, s7, 1
        sub           s7, zero, s6

        @loop for every possible period
        @for(t = t_min@ t <= t_max@ t++)
        @s7 --- scale
