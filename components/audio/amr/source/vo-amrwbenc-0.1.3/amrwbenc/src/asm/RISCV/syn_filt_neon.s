Syn_filt_asm:
	addi sp, sp, -700
	sw ra, 696(sp)
	sw s0, 692(sp)
	sw s1, 688(sp)
	sw s2, 684(sp)
	sw s3, 680(sp)
	sw s4, 676(sp)
	sw s5, 672(sp)
	sw s6, 668(sp)
	sw s7, 664(sp)
	sw s8, 660(sp)
	sw s9, 656(sp)
	sw s10, 652(sp)
	sw s11, 648(sp)
	sw a0, 644(sp)
	sw a1, 640(sp)
	sw a2, 636(sp)
	sw a3, 632(sp)
	sw a4, 628(sp)
	sw a5, 624(sp)
	sw a6, 620(sp)
	sw a7, 616(sp)
	mv s0, a3
	mv s1, sp
	mv s2, a0
	mv s3, s0
	lw a0, 0(s2)
	mv s4, zero
	srai a0, a0, 1
	mv s5, a0
	vmv.v.x v8, s5
	addi a0, a0, 2
	vle16.v v0, (a0)
	vle16.v v1, (a0+16)
	vle16.v v2, (a0+32)
	vle16.v v3, (a0+48)
	vrev64.v v0, v0
	vrev64.v v1, v1
	vrev64.v v2, v2
	vrev64.v v3, v3
	mv s8, zero
	mv s10, s1
	addi a0, s1, 32
	vle16.v v4, (s10)
SYN_LOOP:
	vle16.v v6, (a1)
	vmulh.vv v10, v6, v8
	vmlal.vv v5, v3, v4
	vext.vb v4, v4, v5, 2
	vext.vb v5, v5, v6, 2
	vext.vb v6, v6, v7, 2
	vpadd.vw v12, v10, v11
	addi s8, s8, 1
	vpadd.vw v10, v12, v12
	vmv.v.x v7, v10[0]
	vsub.vv v9, v10, v7
	vsh.vi v20, v9, 12
	vse16.v v20, (a2)
	addi a2, a2, 2
	vse16.v v20, (s10)
	blt SYN_LOOP
	addi s5, s1, 160
	vle16.v v0, (s5)
	vle16.v v1, (s5+16)
	vle16.v v2, (s5+32)
	vle16.v v3, (s5+48)
	vse16.v v0, (s0)
	vse16.v v1, (s0+16)
	vse16.v v2, (s0+32)
	vse16.v v3, (s0+48)
	addi sp, sp, 700
	lw ra, 696(sp)
	lw s0, 692(sp)
	lw s1, 688(sp)
	lw s2, 684(sp)
	lw s3, 680(sp)
	lw s4, 676(sp)
	lw s5, 672(sp)
	lw s6, 668(sp)
	lw s7, 664(sp)
	lw s8, 660(sp)
	lw s9, 656(sp)
	lw s10, 652(sp)
	lw s11, 648(sp)
	lw a0, 644(sp)
	lw a1, 640(sp)
	lw a2, 636(sp)
	lw a3, 632(sp)
	lw a4, 628(sp)
	lw a5, 624(sp)
	lw a6, 620(sp)
	lw a7, 616(sp)
	addi sp, sp, 700
	ret

