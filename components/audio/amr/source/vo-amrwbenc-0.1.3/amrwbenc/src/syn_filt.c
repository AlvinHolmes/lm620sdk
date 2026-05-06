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
*       File: syn_filt.c                                               *
*                                                                      *
*       Description: Do the synthesis filtering 1/A(z)                 *
*                                                                      *
************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "math_op.h"
#include "cnst.h"

void Syn_filt(
		Word16 a[],                           /* (i) Q12 : a[m+1] prediction coefficients           */
		Word16 x[],                           /* (i)     : input signal                             */
		Word16 y[],                           /* (o)     : output signal                            */
		Word16 lg,                            /* (i)     : size of filtering                        */
		Word16 mem[],                         /* (i/o)   : memory associated with this filtering.   */
		Word16 update                         /* (i)     : 0=no update, 1=update of memory.         */
	     )
{
	Word32 i, a0;
	Word16 y_buf[L_SUBFR16k + M16k];
	Word32 L_tmp;
	Word16 *yy, *p1, *p2;
	yy = &y_buf[0];
	/* copy initial filter states into synthesis buffer */
	for (i = 0; i < 16; i++)
	{
		*yy++ = mem[i];
	}
	a0 = (a[0] >> 1);                     /* input / 2 */
#if 0
    Word16 x0,x1,x2,x3;
    Word16 y0,y1,y2,y3;
    Word32 s0,s1,s2,s3;
    Word16 *px;

    px = &x[0];
    for (i = 0; i < lg;)
    {
        p1 = &a[1];
        p2 = &yy[i - 1 + 3];

        s0 = a0 * (*px++);
        s1 = a0 * (*px++);
        s2 = a0 * (*px++);
        s3 = a0 * (*px++);

        //1
        x0 = *p1++;
        x1 = *p1++;
        x2 = *p1++;
        x3 = *p1++;

        y0 = *p2--;
        y1 = *p2--;
        y2 = *p2--;
        y3 = *p2--;

        s3 -= (x3*y3);
        s2 -= (x2*y3);
        s1 -= (x1*y3);
        s0 -= (x0*y3);

        y0 = *p2--;
        y1 = *p2--;
        y2 = *p2--;
        y3 = *p2--;
        
        s2 -= (x3*y0);
        s1 -= (x2*y0 + x3*y1);
        s0 -= (x1*y0 + x2*y1 + x3*y2);

        //2

        x0 = *p1++;
        x1 = *p1++;
        x2 = *p1++;
        x3 = *p1++;

        s3 -= (x0*y0 + x1*y1 + x2*y2 + x3*y3);
        s2 -= (x0*y1 + x1*y2 + x2*y3);
        s1 -= (x0*y2 + x1*y3);
        s0 -= (x0*y3);

        y0 = *p2--;
        y1 = *p2--;
        y2 = *p2--;
        y3 = *p2--;
        
        s2 -= (x3*y0);
        s1 -= (x2*y0 + x3*y1);
        s0 -= (x1*y0 + x2*y1 + x3*y2);

        //3
        x0 = *p1++;
        x1 = *p1++;
        x2 = *p1++;
        x3 = *p1++;

        s3 -= (x0*y0 + x1*y1 + x2*y2 + x3*y3);
        s2 -= (x0*y1 + x1*y2 + x2*y3);
        s1 -= (x0*y2 + x1*y3);
        s0 -= (x0*y3);

        y0 = *p2--;
        y1 = *p2--;
        y2 = *p2--;
        y3 = *p2--;
        
        s2 -= (x3*y0);
        s1 -= (x2*y0 + x3*y1);
        s0 -= (x1*y0 + x2*y1 + x3*y2);

        //4
        x0 = *p1++;
        x1 = *p1++;
        x2 = *p1++;
        x3 = *p1++;

        s3 -= (x0*y0 + x1*y1 + x2*y2 + x3*y3);
        s2 -= (x0*y1 + x1*y2 + x2*y3);
        s1 -= (x0*y2 + x1*y3);
        s0 -= (x0*y3);

        y0 = *p2--;
        y1 = *p2--;
        y2 = *p2--;
        y3 = *p2--;
        
        s2 -= (x3*y0);
        s1 -= (x2*y0 + x3*y1);
        s0 -= (x1*y0 + x2*y1 + x3*y2);

        s1 -=(s0*a[1]);
        s2 -=(s0*a[2] + s1*a[1]);
        s3 -=(s0*a[3] + s1*a[2] + s2*a[1]);

        s0 = L_shl2(s0, 4);
        y[i] = yy[i] = extract_h(L_add(s0, 0x8000)); 
        i++;   
        s1 = L_shl2(s1, 4);
        y[i] = yy[i] = extract_h(L_add(s1, 0x8000));  
        i++;
        s2 = L_shl2(s2, 4);
        y[i] = yy[i] = extract_h(L_add(s2, 0x8000));  
        i++;
        s3 = L_shl2(s3, 4);
        y[i] = yy[i] = extract_h(L_add(s3, 0x8000));   
        i++;
    }

#else
	/* Do the filtering. */
	for (i = 0; i < lg; i++)
	{
		p1 = &a[1];
		p2 = &yy[i-1];
		L_tmp  = vo_mult32(a0, x[i]);
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1++), (*p2--));
		L_tmp -= vo_mult32((*p1), (*p2));

		L_tmp = L_shl2(L_tmp, 4);
		y[i] = yy[i] = extract_h(L_add(L_tmp, 0x8000));
	}
#endif

	/* Update memory if required */
	if (update)
		for (i = 0; i < 16; i++)
		{
			mem[i] = yy[lg - 16 + i];
		}
	return;
}


void Syn_filt_32(
		Word16 a[],                           /* (i) Q12 : a[m+1] prediction coefficients */
		Word16 m,                             /* (i)     : order of LP filter             */
		Word16 exc[],                         /* (i) Qnew: excitation (exc[i] >> Qnew)    */
		Word16 Qnew,                          /* (i)     : exc scaling = 0(min) to 8(max) */
		Word16 sig_hi[],                      /* (o) /16 : synthesis high                 */
		Word16 sig_lo[],                      /* (o) /16 : synthesis low                  */
		Word16 lg                             /* (i)     : size of filtering              */
		)
{
	Word32 i,a0;
	Word32 L_tmp, L_tmp1;
	Word16 *p1, *p2, *p3;
	a0 = a[0] >> (4 + Qnew);          /* input / 16 and >>Qnew */
	/* Do the filtering. */
	for (i = 0; i < lg; i++)
	{
		L_tmp  = 0;
		L_tmp1 = 0;
		p1 = a;
		p2 = &sig_lo[i - 1];
		p3 = &sig_hi[i - 1];

		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));
		L_tmp  -= vo_mult32((*p2--), (*p1));
		L_tmp1 -= vo_mult32((*p3--), (*p1++));

		L_tmp = L_tmp >> 11;
		L_tmp += vo_L_mult(exc[i], a0);

		/* sig_hi = bit16 to bit31 of synthesis */
		L_tmp = L_tmp - (L_tmp1<<1);

		L_tmp = L_tmp >> 3;           /* ai in Q12 */
		sig_hi[i] = extract_h(L_tmp);

		/* sig_lo = bit4 to bit15 of synthesis */
		L_tmp >>= 4;           /* 4 : sig_lo[i] >> 4 */
		sig_lo[i] = (Word16)((L_tmp - (sig_hi[i] << 13)));
	}

	return;
}




