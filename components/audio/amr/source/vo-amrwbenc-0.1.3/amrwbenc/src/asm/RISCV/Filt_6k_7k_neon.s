@/*
@ ** Copyright 2003-2010, VisualOn, Inc.
@ **
@ ** Licensed under the Apache License, Version 2.0 (the "License");
@ ** you may not use this file except in compliance with the License.
@ ** You may obtain a copy of the License at
@ **
@ **     http://www.apache.org/licenses/LICENSE-2.0
@ **
@ ** Unless required by applicable law or agreed to in writing, software
@ ** distributed under the License is distributed on an "AS IS" BASIS,
@ ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@ ** See the License for the specific language governing permissions and
@ ** limitations under the License.
@ */
@
@**********************************************************************/
@void Filt_6k_7k(
@     Word16 signal[],                      /* input:  signal                  */
@     Word16 lg,                            /* input:  length of input         */
@     Word16 mem[]                          /* in/out: memory (size=30)        */
@)
@***********************************************************************
@ r0    ---  signal[]
@ r1    ---  lg
@ r2    ---  mem[]

          .section  .text
          .global   Filt_6k_7k_asm
          .extern   voAWB_fir_6k_7k

Filt_6k_7k_asm:

          addi   		sp, sp, -240
          sd     		s0, 0(sp)
          sd     		s1, 8(sp)
          sd     		s2, 16(sp)
          sd     		s3, 24(sp)
          sd     		s4, 32(sp)
          sd     		s5, 40(sp)
          sd     		s6, 48(sp)
          sd     		s7, 56(sp)
          sd     		s8, 64(sp)
          sd     		s9, 72(sp)
          sd     		s10, 80(sp)
          sd     		s11, 88(sp)
          sd     		t0, 96(sp)
          sd     		t1, 104(sp)
          sd     		t2, 112(sp)
          sd     		t3, 120(sp)
          sd     		t4, 128(sp)
          sd     		t5, 136(sp)
          sd     		t6, 144(sp)

          addi     		t0, a0, 0
          addi     		t1, a2, 0

	  lv.w                v0, 0(t0)
	  lv.w                v1, 8(t0)
	  lv.w                v2, 16(t0)
	  lv.w                v3, 24(t0)

	  sv.w                v0, 0(t1)
	  sv.w                v1, 8(t1)
	  sv.w                v2, 16(t1)
	  sv.w                v3, 24(t1)

          la     		t2, Lable1                  @ get fir_7k address
          lw    		t3, 0(t2)
          add    		t3, t2
          mv                   t2, a0                      @ change myMemCopy to Copy, due to Copy will change r3 content
          add     	    	t4, a2, 60                @ get x[L_FIR - 1] address
          mv           	t5, t2                      @ get signal[i]
          @for (i = lg - 1@ i >= 0@ i--)
          @{
          @     x[i + L_FIR - 1] = signal[i] >> 2@
          @}
	  lv.w                v0, 0(t5)		    @ signal[0]  ~ signal[15]
	  lv.w                v1, 8(t5)             @ signal[16] ~ signal[31]
          lv.w                v2, 16(t5)             @ signal[32] ~ signal[47]
	  lv.w                v3, 24(t5)             @ signal[48] ~ signal[63]
	  sra.w                v4, v0, 2
          sra.w                v5, v1, 2
          sra.w                v6, v2, 2
	  sra.w                v7, v3, 2
	  sv.w                v4, 0(t4)
	  sra.w                v0, v2, 2
	  sra.w                v1, v3, 2
	  sra.w                v4, v4, 2
	  sra.w                v5, v5, 2
	  sra.w                v6, v6, 2
	  sra.w                v7, v7, 2
	  sv.w                v5, 8(t4)
	  sra.w                v2, v4, 2
	  sra.w                v3, v5, 2
	  sra.w                v4, v6, 2
	  sra.w                v5, v7, 2
	  sv.w                v6, 16(t4)
	  sv.w                v7, 24(t4)
	  sv.w                v0, 32(t4)
	  sv.w                v1, 40(t4)
	  sv.w                v2, 48(t4)
	  sv.w                v3, 56(t4)
	  sv.w                v4, 64(t4)
	  sv.w                v5, 72(t4)

	  mv                   s7, a2
          @STR     		r5, [sp, #-4]               @ PUSH  r5
