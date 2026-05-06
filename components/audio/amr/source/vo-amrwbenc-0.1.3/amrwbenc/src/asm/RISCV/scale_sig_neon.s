# void Scale_sig(Word16 x[], Word16 lg, Word16 exp)
Scale_sig:
    addi sp, sp, -16
    sd ra, 0(sp)
    sd s0, 8(sp)
    addi s0, zero, 4
    li t0, 0x8000
    vsetvli t1, t0, e16, m1
    vle16.v v15, (zero)
    vsetvli t1, t0, e32, m1
    vslideup.vi v14, v0, exp
    mv t2, a0
    blt lg, 64, LOOP
    beq lg, 128, LOOP
    beq lg, 256, LOOP
    beq lg, 80, LOOP1

LOOP1:
    vle16.v v0, (t2)
    vle16.v v1, (t2+16)
    vle16.v v2, (t2+32)
    vle16.v v3, (t2+48)
    vslideup.vi v8, v0, exp
    vslideup.vi v9, v1, exp
    vslideup.vi v10, v2, exp
    vslideup.vi v11, v3, exp
    vadd.vi v8, v8, v15
    vadd.vi v9, v9, v15
    vadd.vi v10, v10, v15
    vadd.vi v11, v11, v15
    vse16.v v8, (a0)
    vse16.v v9, (a0+16)
    addi a0, a0, 32
    addi t2, t2, 64
    b LOOP1

LOOP:
    vle16.v v0, (t2)
    vle16.v v1, (t2+16)
    vle16.v v2, (t2+32)
    vle16.v v3, (t2+48)
    vle16.v v4, (t2+64)
    vle16.v v5, (t2+80)
    vle16.v v6, (t2+96)
    vle16.v v7, (t2+112)
    vslideup.vi v8, v0, exp
    vslideup.vi v9, v1, exp
    vslideup.vi v10, v2, exp
    vslideup.vi v11, v3, exp
    vslideup.vi v12, v4, exp
    vslideup.vi v13, v5, exp
    vslideup.vi v14, v6, exp
    vslideup.vi v15, v7, exp
    vadd.vi v8, v8, v15
    vadd.vi v9, v9, v15
    vadd.vi v10, v10, v15
    vadd.vi v11, v11, v15
    vadd.vi v12, v12, v15
    vadd.vi v13, v13, v15
    vadd.vi v14, v14, v15
    vadd.vi v15, v15, v15
    vse16.v v8, (a0)
    vse16.v v9, (a0+16)
    vse16.v v10, (a0+32)
    vse16.v v11, (a0+48)
    vse16.v v12, (a0+64)
    vse16.v v13, (a0+80)
    vse16.v v14, (a0+96)
    vse16.v v15, (a0+112)
    addi a0, a0, 128
    addi t2, t2, 128
    blt lg, 64, LOOP
    vle16.v v0, (t2)
    vle16.v v1, (t2+16)
    vle16.v v2, (t2+32)
    vle16.v v3, (t2+48)
    vslideup.vi v8, v0, exp
    vslideup.vi v9, v1, exp
    vslideup.vi v10, v2, exp
    vslideup.vi v11, v3, exp
    vadd.vi v8, v8, v15
    vadd.vi v9, v9, v15
    vadd.vi v10, v10, v15
    vadd.vi v11, v11, v15
    vse16.v v8, (a0)
    vse16.v v9, (a0+16)
    vse16.v v10, (a0+32)
    vse16.v v11, (a0+48)

Scale_sig_asm_end:
    ld ra, 0(sp)
    ld s0, 8(sp)
    addi sp, sp, 16
    ret


