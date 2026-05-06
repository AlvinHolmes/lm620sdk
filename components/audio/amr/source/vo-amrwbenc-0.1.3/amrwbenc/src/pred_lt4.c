/*
 ** Copyright 2003-2010, VisualOn, Inc.
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

/***********************************************************************
*      File: pred_lt4.c                                                *
*                                                                      *
*      Description: Compute the result of long term prediction with    *
*      fractional interpolation of resolution 1/4                      *
*      on return exc[0..L_subr-1] contains the interpolated signal     *
*      (adaptive codebook excitation)                                  *
*                                                                      *
************************************************************************/

#include "typedef.h"
#include "basic_op.h"

#define UP_SAMP      4
#define L_INTERPOL2  16

/* 1/4 resolution interpolation filter (-3 dB at 0.856*fs/2) in Q14 */

Word16 inter4_2[4][32] =
{
	{0,-2,4,-2,-10,38,-88,165,-275,424,-619,871,-1207,1699,-2598,5531,14031,-2147,780,-249,
	-16,153,-213,226,-209,175,-133,91,-55,28,-10,2},

	{1,-7,19,-33,47,-52,43,-9,-60,175,-355,626,-1044,1749,-3267,10359,10359,-3267,1749,-1044,
	626,-355,175,-60,-9,43,-52,47,-33,19, -7, 1},

	{2,-10,28,-55,91,-133,175,-209,226,-213,153,-16,-249,780,-2147,14031,5531,-2598,1699,-1207,
	871,-619,424,-275,165,-88,38,-10,-2,4,-2,0},

	{1,-7,22,-49,92,-153,231,-325,431,-544,656,-762,853,-923,968,15401,968,-923,853,-762,
	656,-544,431,-325,231,-153,92,-49,22,-7, 1, 0}

};

void Pred_lt4(
		Word16 exc[],                         /* in/out: excitation buffer */
		Word16 T0,                            /* input : integer pitch lag */
		Word16 frac,                          /* input : fraction of lag   */
		Word16 L_subfr                        /* input : subframe size     */
	     )
{
	Word16 j, k, *x;
	Word32 L_sum;
	Word16 *ptr, *ptr1;
	Word16 *ptr2;

	x = exc - T0;
	frac = -frac;
	if (frac < 0)
	{
		frac += UP_SAMP;
		x--;
	}
	x -= 15;                                     /* x = L_INTERPOL2 - 1 */
	k = 3 - frac;                                /* k = UP_SAMP - 1 - frac */

	ptr2 = &(inter4_2[k][0]);

#ifdef RISCV_DSP
    Word16 x0,x1,x2,x3;
    Word16 k0,k1,k2,k3;
    Word32 s0,s1,s2,s3;

    Word8 i;

    for (j = 0; j < L_subfr -1;)
    {
        ptr1 = (x + j);
        ptr = ptr2;
        s0=s1=s2=s3=0;

        x0 = *ptr1++;
        x1 = *ptr1++;
        x2 = *ptr1++;
        x3 = *ptr1++;

        i= 0;
        while(i < 8)
        {
            k0 = *ptr++;
            k1 = *ptr++;
            k2 = *ptr++;
            k3 = *ptr++;

            s0 += k0*x0 + k1*x1 + k2*x2 + k3*x3;
            s1 += k0*x1 + k1*x2 + k2*x3;
            s2 += k0*x2 + k1*x3;
            s3 += k0*x3;

            x0 = *ptr1++;
            x1 = *ptr1++;
            x2 = *ptr1++;
            x3 = *ptr1++;

            s1 += k3*x0;
            s2 += k3*x1 + k2*x0;
            s3 += k3*x2 + k2*x1 + k1*x0;

            i ++;
        }

        s0 = L_shl2(s0, 2);
		exc[j++] = extract_h(L_add(s0, 0x8000));
        s1 = L_shl2(s1, 2);
		exc[j++] = extract_h(L_add(s1, 0x8000));
        s2 = L_shl2(s2, 2);
		exc[j++] = extract_h(L_add(s2, 0x8000));
        s3 = L_shl2(s3, 2);
		exc[j++] = extract_h(L_add(s3, 0x8000));
    }

        ptr = ptr2;
		ptr1 = x + L_subfr - 1;
		L_sum  = vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));

		L_sum = L_shl2(L_sum, 2);
		exc[L_subfr-1] = extract_h(L_add(L_sum, 0x8000));

#else
	for (j = 0; j < L_subfr; j++)
	{
		ptr = ptr2;
		ptr1 = x;
		L_sum  = vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));
		L_sum += vo_mult32((*ptr1++), (*ptr++));

		L_sum = L_shl2(L_sum, 2);
		exc[j] = extract_h(L_add(L_sum, 0x8000));
		x++;
	}
#endif
	return;
}



