#include "psinc/handlers/DataHandler.h"

#include <emergent/Maths.h>

using namespace std;
using namespace emergent;


namespace psinc
{
	void DataHandler::DecodeMono(byte *src, byte *dst, int width, int height, int depth)
	{
		int size = width * height;

		if (depth == 3)
		{
			for (int i=0; i<size; i++)
			{
				*dst++ = *src;
				*dst++ = *src;
				*dst++ = *src++;
			}
		}
		else memcpy(dst, src, size);
	}


	void DataHandler::DecodeGrey(byte *src, byte *dst, int width, int height)
	{
		int sw		= width;
		int sh		= height;
		int dw		= sw - 4;
		int dh		= sh - 4;
		byte *pg	= src + (sw << 1) + 2;
		byte *pa	= pg - (sw << 1);
		byte *pc	= pg - sw;
		byte *pb	= pc - 1;
		byte *pd	= pc + 1;
		byte *pe	= pg - 2;
		byte *pf	= pg - 1;
		byte *ph	= pg + 1;
		byte *pi	= pg + 2;
		byte *pk	= pg + sw;
		byte *pj	= pk - 1;
		byte *pl	= pk + 1;
		byte *pm	= pg + (sw << 1);

		int x, y;
		bool oddPixel, oddLine = false;

		for (y=0; y<dh; y++, pa+=4, pb+=4, pc+=4, pd+=4, pe+=4, pf+=4, pg+=4, ph+=4, pi+=4, pj+=4, pk+=4, pl+=4, pm+=4)
		{
			oddPixel = false;
			for (x=0; x<dw; x++)
			{
				if (oddLine)	// GRGR: R or G
				{
					if (oddPixel)	*dst++ = Maths::clamp<byte>((*pg++ * 36 + ((*pc++ + *pf++ + *ph++ + *pk++ + *pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 5) / 48);
					else			*dst++ = Maths::clamp<byte>((*pg++ * 36 + ((*pf++ + *ph++ + *pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - *pa++ - *pm++ - *pe++ - *pi++) / 48);
				}
				else			// BGBG: G or B
				{
					if (oddPixel)	*dst++ = Maths::clamp<byte>((*pg++ * 36 + ((*pf++ + *ph++ + *pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - *pa++ - *pm++ - *pe++ - *pi++) / 48);
					else			*dst++ = Maths::clamp<byte>((*pg++ * 36 + ((*pc++ + *pf++ + *ph++ + *pk++ + *pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 5) / 48);
				}
				oddPixel = !oddPixel;
			}

			oddLine = !oddLine;
		}
	}


	void DataHandler::DecodeColour(byte *src, byte *dst, int width, int height)
	{
		int sw		= width;
		int sh		= height;
		int dw		= sw - 4;
		int dh		= sh - 4;
		byte *pg	= src + (sw << 1) + 2;
		byte *pa	= pg - (sw << 1);
		byte *pc	= pg - sw;
		byte *pb	= pc - 1;
		byte *pd	= pc + 1;
		byte *pe	= pg - 2;
		byte *pf	= pg - 1;
		byte *ph	= pg + 1;
		byte *pi	= pg + 2;
		byte *pk	= pg + sw;
		byte *pj	= pk - 1;
		byte *pl	= pk + 1;
		byte *pm	= pg + (sw << 1);

		int x, y;
		byte r, g, b;
		bool oddPixel, oddLine = false;

		for (y=0; y<dh; y++, pa+=4, pb+=4, pc+=4, pd+=4, pe+=4, pf+=4, pg+=4, ph+=4, pi+=4, pj+=4, pk+=4, pl+=4, pm+=4)
		{
			oddPixel = false;
			for (x=0; x<dw; x++)
			{
				if (oddLine)	// GRGR: R or G
				{
					if (oddPixel)
					{
						r = *pg;
						g = Maths::clamp<byte>(((*pg << 2) + ((*pc++ + *pf++ + *ph++ + *pk++) << 1) - *pa - *pe - *pi - *pm) >> 3);
						b = Maths::clamp<byte>((*pg++ * 12 + ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 3) >> 4);
					}
					else
					{
						r = Maths::clamp<byte>((*pg * 10 + ((*pf++ + *ph++) << 3) - ((*pb + *pd + *pj + *pl + *pe + *pi) << 1) + *pa + *pm) >> 4);
						g = *pg;
						b = Maths::clamp<byte>((*pg++ * 10 + ((*pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++ + *pa++ + *pm++) << 1) + *pe++ + *pi++) >> 4);
					}
				}
				else			// BGBG: G or B
				{
					if (oddPixel)
					{
						r = Maths::clamp<byte>((*pg * 10 + ((*pc++ + *pk++) << 3) - ((*pb + *pd + *pj + *pl + *pa + *pm) << 1) + *pe + *pi) >> 4);
						g = *pg;
						b = Maths::clamp<byte>((*pg++ * 10 + ((*pf++ + *ph++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++ + *pe++ + *pi++) << 1) + *pa++ + *pm++) >> 4);
					}
					else
					{
						r = Maths::clamp<byte>((*pg * 12 + ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa + *pe + *pi + *pm) * 3) >> 4);
						g = Maths::clamp<byte>(((*pg << 2) + ((*pc++ + *pf++ + *ph++ + *pk++) << 1) - *pa++ - *pe++ - *pi++ - *pm++) >> 3);
						b = *pg++;
					}
				}

				*dst++ = r; *dst++ = g; *dst++ = b;

				oddPixel = !oddPixel;
			}

			oddLine = !oddLine;
		}
	}
}
