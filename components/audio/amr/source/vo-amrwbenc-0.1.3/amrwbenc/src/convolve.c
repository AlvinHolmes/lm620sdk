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
       File: convolve.c

	   Description:Perform the convolution between two vectors x[] and h[]
	               and write the result in the vector y[]

************************************************************************/

#include "typedef.h"
#include "basic_op.h"

void Convolve (
		Word16 x[],        /* (i)     : input vector                           */
		Word16 h[],        /* (i)     : impulse response                       */
		Word16 y[],        /* (o)     : output vector                          */
		Word16 L           /* (i)     : vector size                            */
	      )
{
#ifdef RISCV_DSP
    Word16 x0, x1, x2, x3;
    Word16 h0, h1, h2, h3;
    Word32 s0, s1, s2, s3;
    Word8 i;
    Word16 *px;
    Word16 *ph;
    Word8 j;

    for(i = 0; i < 64;)
    {
        // 先计算斜角
        px = &x[i+3];
        ph = h;

        x0 = *px--;
        x1 = *px--;
        x2 = *px--;
        x3 = *px--;

        h0 = *ph++;
        h1 = *ph++;
        h2 = *ph++;
        h3 = *ph++;

        s3 = x0*h0 + x1*h1 + x2*h2 + x3*h3;
        s2 = x1*h0 + x2*h1 + x3*h2;
        s1 = x2*h0 + x3*h1;
        s0 = x3*h0;

        //再计算方形矩阵
        j= 1;
        ph = &h[j];
        h0 = *ph++;
        h1 = *ph++;
        h2 = *ph++;
        h3 = *ph++;

        while(j <= i)
        {
            x0 = *px--;
            x1 = *px--;
            x2 = *px--;
            x3 = *px--;

            s0 += x0*h0 + x1*h1 + x2*h2 + x3*h3;
            s1 += x0*h1 + x1*h2 + x2*h3;
            s2 += x0*h2 + x1*h3;
            s3 += x0*h3;

            h0 = *ph++;
            h1 = *ph++;
            h2 = *ph++;
            h3 = *ph++;

            s1 += x3*h0;
            s2 += x3*h1 + x2*h0;
            s3 += x3*h2 + x2*h1 + x1*h0;

            j += 4;
        }

        y[i++] = ((s0<<1) + 0x8000)>>16;
        y[i++] = ((s1<<1) + 0x8000)>>16;
        y[i++] = ((s2<<1) + 0x8000)>>16;
        y[i++] = ((s3<<1) + 0x8000)>>16;
    }

#else
	Word32  i, n;
	Word16 *tmpH,*tmpX;
	Word32 s;
	for (n = 0; n < 64;)
	{
		tmpH = h+n;
		tmpX = x;
		i=n+1;
		s = vo_mult32((*tmpX++), (*tmpH--));i--;
		while(i>0)
		{
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			i -= 4;
		}
		y[n] = ((s<<1) + 0x8000)>>16;
		n++;

		tmpH = h+n;
		tmpX = x;
		i=n+1;
		s =  vo_mult32((*tmpX++), (*tmpH--));i--;
		s += vo_mult32((*tmpX++), (*tmpH--));i--;

		while(i>0)
		{
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			i -= 4;
		}
		y[n] = ((s<<1) + 0x8000)>>16;
		n++;

		tmpH = h+n;
		tmpX = x;
		i=n+1;
		s =  vo_mult32((*tmpX++), (*tmpH--));i--;
		s += vo_mult32((*tmpX++), (*tmpH--));i--;
		s += vo_mult32((*tmpX++), (*tmpH--));i--;

		while(i>0)
		{
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			i -= 4;
		}
		y[n] = ((s<<1) + 0x8000)>>16;
		n++;

		s = 0;
		tmpH = h+n;
		tmpX = x;
		i=n+1;
		while(i>0)
		{
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			s += vo_mult32((*tmpX++), (*tmpH--));
			i -= 4;
		}
		y[n] = ((s<<1) + 0x8000)>>16;
		n++;
	}
#endif
	return;
}



