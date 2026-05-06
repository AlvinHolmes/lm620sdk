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
*  File: residu.c                                                      *
*                                                                      *
*  Description: Compute the LPC residual by filtering                  *
*             the input speech through A(z)                            *
*                                                                      *
************************************************************************/

#include "typedef.h"
#include "basic_op.h"

void Residu(
		Word16 a[],                           /* (i) Q12 : prediction coefficients                     */
		Word16 x[],                           /* (i)     : speech (values x[-m..-1] are needed         */
		Word16 y[],                           /* (o) x2  : residual signal                             */
		Word16 lg                             /* (i)     : size of filtering                           */
		)
{

#ifdef RISCV_DSP
	Word16 i,*pa, *px;
	Word32 s0, s1, s2, s3;
	Word16 a0,a1,a2,a3,x0,x1,x2,x3;

	for (i = 0; i < lg; )
	{
		pa = &a[13];
		px = &x[i-16];

		// a13~a16
		a0 = *pa++;
		a1 = *pa++;
		a2 = *pa++;
		a3 = *pa++;
		
		x0 = *px++;
		x1 = *px++;
		x2 = *px++;
		x3 = *px++;

		// reuse a0-a3, only load once
		s0 = vo_mult32(a0, x3);
		s0 += vo_mult32(a1, x2);
		s0 += vo_mult32(a2, x1);
		s0 += vo_mult32(a3, x0);
		// reuse x1-x3, only update one x
		x0 = *px++;

		s1 = vo_mult32(a0, x0);
		s1 += vo_mult32(a1, x3);
		s1 += vo_mult32(a2, x2);
		s1 += vo_mult32(a3, x1);
		x1 = *px++;

		s2 = vo_mult32(a0, x1);
		s2 += vo_mult32(a1, x0);
		s2 += vo_mult32(a2, x3);
		s2 += vo_mult32(a3, x2);
		x2 = *px++;

		s3 = vo_mult32(a0, x2);
		s3 += vo_mult32(a1, x1);
		s3 += vo_mult32(a2, x0);
		s3 += vo_mult32(a3, x3);
		x3 = *px++;

		// ----------------------------------
		// update a for the next matrix
		// ----------------------------------
		// a9~a12
        pa -= 8;
		a0 = *pa++;
		a1 = *pa++;
		a2 = *pa++;
		a3 = *pa++;

		// reuse a0-a3, only load once
		s0 += vo_mult32(a0, x3);
		s0 += vo_mult32(a1, x2);
		s0 += vo_mult32(a2, x1);
		s0 += vo_mult32(a3, x0);
		// reuse x1-x3, only update one x
		x0 = *px++;

		s1 += vo_mult32(a0, x0);
		s1 += vo_mult32(a1, x3);
		s1 += vo_mult32(a2, x2);
		s1 += vo_mult32(a3, x1);
		x1 = *px++;

		s2 += vo_mult32(a0, x1);
		s2 += vo_mult32(a1, x0);
		s2 += vo_mult32(a2, x3);
		s2 += vo_mult32(a3, x2);
		x2 = *px++;

		s3 += vo_mult32(a0, x2);
		s3 += vo_mult32(a1, x1);
		s3 += vo_mult32(a2, x0);
		s3 += vo_mult32(a3, x3);
		x3 = *px++;

		// ----------------------------------
		// update a for the next matrix
		// ----------------------------------
		// a5~a8
        pa -= 8;
		a0 = *pa++;
		a1 = *pa++;
		a2 = *pa++;
		a3 = *pa++;

		// reuse a0-a3, only load once
		s0 += vo_mult32(a0, x3);
		s0 += vo_mult32(a1, x2);
		s0 += vo_mult32(a2, x1);
		s0 += vo_mult32(a3, x0);
		// reuse x1-x3, only update one x
		x0 = *px++;

		s1 += vo_mult32(a0, x0);
		s1 += vo_mult32(a1, x3);
		s1 += vo_mult32(a2, x2);
		s1 += vo_mult32(a3, x1);
		x1 = *px++;

		s2 += vo_mult32(a0, x1);
		s2 += vo_mult32(a1, x0);
		s2 += vo_mult32(a2, x3);
		s2 += vo_mult32(a3, x2);
		x2 = *px++;

		s3 += vo_mult32(a0, x2);
		s3 += vo_mult32(a1, x1);
		s3 += vo_mult32(a2, x0);
		s3 += vo_mult32(a3, x3);
		x3 = *px++;

		// ----------------------------------
		// update a for the next matrix
		// ----------------------------------
		// a1~a4
        pa -= 8;
		a0 = *pa++;
		a1 = *pa++;
		a2 = *pa++;
		a3 = *pa++;

		// reuse a0-a3, only load once
		s0 += vo_mult32(a0, x3);
		s0 += vo_mult32(a1, x2);
		s0 += vo_mult32(a2, x1);
		s0 += vo_mult32(a3, x0);
		// reuse x1-x3, only update one x
		x0 = *px++;

		s1 += vo_mult32(a0, x0);
		s1 += vo_mult32(a1, x3);
		s1 += vo_mult32(a2, x2);
		s1 += vo_mult32(a3, x1);
		x1 = *px++;

		s2 += vo_mult32(a0, x1);
		s2 += vo_mult32(a1, x0);
		s2 += vo_mult32(a2, x3);
		s2 += vo_mult32(a3, x2);
		x2 = *px++;

		s3 += vo_mult32(a0, x2);
		s3 += vo_mult32(a1, x1);
		s3 += vo_mult32(a2, x0);
		s3 += vo_mult32(a3, x3);
		x3 = *px++;

		// ---------------------------------
		// update a for the next matrix, the last matrix
		// ----------------------------------
		a0 = *(pa-5);
		// reuse a0-a3, only load once
		s0 += vo_mult32(a0, x0);
		s1 += vo_mult32(a0, x1);
		s2 += vo_mult32(a0, x2);
		s3 += vo_mult32(a0, x3);
    
		s0 = L_shl2(s0, 5);
		s1 = L_shl2(s1, 5);
		s2 = L_shl2(s2, 5);
		s3 = L_shl2(s3, 5);
		y[i++] = extract_h(L_add(s0, 0x8000));
		y[i++] = extract_h(L_add(s1, 0x8000));
		y[i++] = extract_h(L_add(s2, 0x8000));
		y[i++] = extract_h(L_add(s3, 0x8000));
	}

#else
	Word16 i,*p1, *p2;
	Word32 s;
	for (i = 0; i < lg; i++)
	{
		p1 = a;
		p2 = &x[i];
		s  = vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1++), (*p2--));
		s += vo_mult32((*p1), (*p2));

		s = L_shl2(s, 5);
		y[i] = extract_h(L_add(s, 0x8000));
	}
#endif
	return;
}
