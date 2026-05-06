.section .text
.global pred_lt4_asm
.extern voAWB_inter4_2

pred_lt4_asm:
	addi sp, sp, -32
	sw ra, 28(sp)
	sw s0, 24(sp)
	sw s1, 20(sp)
	sw s2, 16(sp)
	sw s3, 12(sp)
	sw s4, 8(sp)
	sw s5, 4(sp)
	sw s6, 0(sp)

	sub s0, a0, a1
	addi a2, a2, -1
	addi s0, s0, -30
	blt a2, zero, L1
	addi a2, a2, 4
	addi s0, s0, -2

L1:
	la s4, voAWB_inter4_2
	lw s5, 0(s4)
	addi s5, s5, voAWB_inter4_2
	addi a2, a2, -3
	slli s6, a2, 1
	add s5, s5, s6
	vlw.v v0, (s5)
	addi s5, s5, 16
	vlw.v v1, (s5)
	addi s5, s5, 16
	vlw.v v2, (s0)
	addi s0, s0, 16
	vlw.v v3, (s0)
	addi s0, s0, 16

L2:
	vdot.vv v4, v0, v2
	vdot.vv v5, v1, v2
	vdot.vv v6, v0, v3
	vdot.vv v7, v1, v3
	vlw.v v2, (s0)
	addi s0, s0, 16
	vext.16 v0, v0, v1, 2
	vext.16 v1, v1, v2, 2
	vext.16 v2, v2, v3, 2
	vext.16 v3, v3, v4, 2
	vmv.v.x s1, v4
	vaddw.s16 s1, s1, s1
	addi s5, s5, 1
	vaddw.s16 s1, s1, s1
	addi s5, s5, 1
	addi s4, s4, 1
	addi s6, s6, 1
	blt s6, a3, L2

L3:
	vmv.v.x s1, v5
	vaddw.s16 s1, s1, s1
	vaddw.s16 s1, s1, s1
	vmv.v.x s2, v6
	vaddw.s16 s2, s2, s2
	vaddw.s16 s2, s2, s2
	vmv.v.x s3, v7
	vaddw.s16 s3, s3, s3
	vaddw.s16 s3, s3, s3
	addi s1, s1, 0x8000
	addi s2, s2, 0x8000
	addi s3, s3, 0x8000
	srai s1, s1, 16
	srai s2, s2, 16
	srai s3, s3, 16
	sh s1, 0(a0)
	addi a0, a0, 2
	sh s2, 0(a0)
	addi a0, a0, 2
	sh s3, 0(a0)
	addi a0, a0, 2

L4:
	lw ra, 28(sp)
	lw s0, 24(sp)
	lw s1, 20(sp)
	lw s2, 16(sp)
	lw s3, 12(sp)
	lw s4, 8(sp)
	lw s5, 4(sp)
	lw s6, 0(sp)
	addi sp, sp, 32
	ret


