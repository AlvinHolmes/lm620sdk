Syn_filt_32_asm:
	addi sp, sp, -64
	sd ra, 56(sp)
	sd s0, 48(sp)
	sd s1, 40(sp)
	sd s2, 32(sp)
	sd s3, 24(sp)
	sd s4, 16(sp)
	sd s5, 8(sp)
	sd s6, 0(sp)

	lw s4, 0(a0)
	addi s5, a3, 4
	slli s6, a3, 1
	addi s6, s6, 32
	addi a0, a0, 2
	addi s0, a1, 1
	addi s1, s4, 4
	addi s2, a2, 2
	addi s3, a2, 4

syn_loop:
	lw s4, 0(a0)
	addi a0, a0, 2
	lw s7, 0(s1)
	lw s8, 0(s1)
	lw s9, 0(s1)
	lw s10, 0(s1)
	addi s1, s1, 4
	lw s11, -32(s2)
	lw s12, -28(s2)
	lw s13, -24(s2)
	lw s14, -20(s2)
	lw s15, -16(s2)
	lw s16, -12(s2)
	lw s17, -8(s2)
	lw s18, -4(s2)
	addi s2, s2, 4

	mul s19, s4, s0
	addi s0, s0, 1
	sub s20, x0, s19
	mul s21, s7, s3
	mul s22, s8, s2
	mul s23, s9, s1
	mul s24, s10, s4
	add s25, s21, s22
	add s25, s25, s23
	add s25, s25, s24
	addi s3, s3, 1
	addi s4, s4, 1
	addi s5, s5, 2
	addi s6, s6, 2
	sw s20, -2(s5)
	sw s25, -2(s6)

	blt s0, a6, syn_loop

	addi sp, sp, 64
	ld ra, 56(sp)
	ld s0, 48(sp)
	ld s1, 40(sp)
	ld s2, 32(sp)
	ld s3, 24(sp)
	ld s4, 16(sp)
	ld s5, 8(sp)
	ld s6, 0(sp)
	jr ra


