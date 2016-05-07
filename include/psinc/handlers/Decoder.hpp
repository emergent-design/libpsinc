#pragma once

#include <emergent/Maths.hpp>
#include <future>


namespace psinc
{
	/// Helper class for Bayer decoding images.
	class Decoder
	{
		public:

			// Decode data from a monochrome sensor
			template <typename T, typename U> static void Mono(T *src, U *dst, int width, int height, int depth, uint16_t shift)
			{
				using emg::Maths;

				int size = width * height;

				if (depth == 3)
				{
					U value;
					for (int i=0; i<size; i++)
					{
						value	= sizeof(U) < sizeof(T) ? Maths::clamp<U>(*src++ >> shift) : *src++;
						*dst++	= value;
						*dst++	= value;
						*dst++	= value;
					}
				}
				else if (std::is_same<T, U>::value)
				{
					memcpy(dst, src, size * sizeof(T));
				}
				else
				{
					for (int i=0; i<size; i++)
					{
						*dst++ = sizeof(U) < sizeof(T) ? Maths::clamp<U>(*src++ >> shift) : *src++;
					}
				}
			}


			// Decode data from  a bayer sensor to a greyscale image
			// Bayer mode offsets:
			//		0: 0, 0
			//		1: 0, 1
			//		2: 1, 0
			//		3: 1, 1
			/*template <typename T, typename U> static void BayerGreyOld(T *src, U *dst, int width, int height, byte bayerMode, uint16_t shift)
			{
				using emg::Maths;

				int sw	= width;
				int sh	= height;
				int dw	= sw - 4;
				int dh	= sh - 4;
				T *pg	= src + (sw << 1) + 2;
				T *pa	= pg - (sw << 1);
				T *pc	= pg - sw;
				T *pb	= pc - 1;
				T *pd	= pc + 1;
				T *pe	= pg - 2;
				T *pf	= pg - 1;
				T *ph	= pg + 1;
				T *pi	= pg + 2;
				T *pk	= pg + sw;
				T *pj	= pk - 1;
				T *pl	= pk + 1;
				T *pm	= pg + (sw << 1);

				std::function<U(long)> 			clamp = [](long value) { return Maths::clamp<U>(value); };
				if (!std::is_same<T, U>::value) clamp = [&](long value) { return Maths::clamp<U>(value >> shift); };

				int x, y;
				bool oddPixel, oddLine = bayerMode % 2;

				for (y=0; y<dh; y++, pa+=4, pb+=4, pc+=4, pd+=4, pe+=4, pf+=4, pg+=4, ph+=4, pi+=4, pj+=4, pk+=4, pl+=4, pm+=4)
				{
					oddPixel = bayerMode > 1;
					for (x=0; x<dw; x++)
					{
						if (oddLine)	// GRGR: R or G
						{
							if (oddPixel)	*dst++ = clamp((*pg++ * 36 + ((*pc++ + *pf++ + *ph++ + *pk++ + *pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 5) / 48);
							else			*dst++ = clamp((*pg++ * 36 + ((*pf++ + *ph++ + *pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - *pa++ - *pm++ - *pe++ - *pi++) / 48);
						}
						else			// BGBG: G or B
						{
							if (oddPixel)	*dst++ = clamp((*pg++ * 36 + ((*pf++ + *ph++ + *pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - *pa++ - *pm++ - *pe++ - *pi++) / 48);
							else			*dst++ = clamp((*pg++ * 36 + ((*pc++ + *pf++ + *ph++ + *pk++ + *pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 5) / 48);
						}
						oddPixel = !oddPixel;
					}

					oddLine = !oddLine;
				}
			}*/


			// Decode data from a bayer sensor to an RGB image
			// Bayer mode offsets:
			//		0: 0, 0
			//		1: 0, 1
			//		2: 1, 0
			//		3: 1, 1
			/*template <typename T, typename U> static void BayerColourOld(T *src, U *dst, int width, int height, byte bayerMode, uint16_t)
			{
				using emg::Maths;

				int sw	= width;
				int sh	= height;
				int dw	= sw - 4;
				int dh	= sh - 4;
				T *pg	= src + (sw << 1) + 2;
				T *pa	= pg - (sw << 1);
				T *pc	= pg - sw;
				T *pb	= pc - 1;
				T *pd	= pc + 1;
				T *pe	= pg - 2;
				T *pf	= pg - 1;
				T *ph	= pg + 1;
				T *pi	= pg + 2;
				T *pk	= pg + sw;
				T *pj	= pk - 1;
				T *pl	= pk + 1;
				T *pm	= pg + (sw << 1);

				//std::function<U(long)> 			clamp = [](long value) { return Maths::clamp<U>(value); };
				//if (!std::is_same<T, U>::value) clamp = [&](long value) { return Maths::clamp<U>(value >> shift); };

				int x, y;
				U r, g, b;
				bool oddPixel, oddLine = bayerMode % 2;

				for (y=0; y<dh; y++, pa+=4, pb+=4, pc+=4, pd+=4, pe+=4, pf+=4, pg+=4, ph+=4, pi+=4, pj+=4, pk+=4, pl+=4, pm+=4)
				{
					oddPixel = bayerMode > 1;
					for (x=0; x<dw; x++)
					{
						if (oddLine)	// GRGR: R or G
						{
							if (oddPixel)
							{
								r = Maths::clamp<U>(*pg);
								g = Maths::clamp<U>(((*pg << 2) + ((*pc++ + *pf++ + *ph++ + *pk++) << 1) - *pa - *pe - *pi - *pm) >> 3);
								b = Maths::clamp<U>((*pg++ * 12 + ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 3) >> 4);
							}
							else
							{
								r = Maths::clamp<U>((*pg * 10 + ((*pf++ + *ph++) << 3) - ((*pb + *pd + *pj + *pl + *pe + *pi) << 1) + *pa + *pm) >> 4);
								g = Maths::clamp<U>(*pg);
								b = Maths::clamp<U>((*pg++ * 10 + ((*pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++ + *pa++ + *pm++) << 1) + *pe++ + *pi++) >> 4);
							}
						}
						else			// BGBG: G or B
						{
							if (oddPixel)
							{
								r = Maths::clamp<U>((*pg * 10 + ((*pc++ + *pk++) << 3) - ((*pb + *pd + *pj + *pl + *pa + *pm) << 1) + *pe + *pi) >> 4);
								g = Maths::clamp<U>(*pg);
								b = Maths::clamp<U>((*pg++ * 10 + ((*pf++ + *ph++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++ + *pe++ + *pi++) << 1) + *pa++ + *pm++) >> 4);
							}
							else
							{
								r = Maths::clamp<U>((*pg * 12 + ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa + *pe + *pi + *pm) * 3) >> 4);
								g = Maths::clamp<U>(((*pg << 2) + ((*pc++ + *pf++ + *ph++ + *pk++) << 1) - *pa++ - *pe++ - *pi++ - *pm++) >> 3);
								b = Maths::clamp<U>(*pg++);
							}
						}

						*dst++ = r; *dst++ = g; *dst++ = b;

						oddPixel = !oddPixel;
					}

					oddLine = !oddLine;
				}
			}*/


			//       -1
			//       +2
			// -1 +2 +4 +2 -1		Divisor of 8
			//       +2
			//       -1
			template <typename T> static inline int BayerCross(T *src, const int w, const int w2)
			{
				return (4 * src[0] + 2 * (src[-w] + src[w] + src[-1] + src[+1])  - src[-w2] - src[w2] - src[-2] - src[+2]) >> 3;
			}


			//        -3
			//    +4     +4
			// -3    +12    -3		Divisor of 16
			//    +4     +4
			//        -3
			template <typename T> static inline int BayerChecker(T *src, const int w, const int w2)
			{
				return (12 * src[0] + 4 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1]) - 3 * (src[-w2] + src[w2] + src[-2] + src[2])) >> 4;
			}


			//         1
			//    -2     -2
			// -2 +8 +10 +8 -2		Divisor of 16
			//    -2     -2
			//         1
			template <typename T> static inline int BayerTheta(T *src, const int w, const int w2)
			{
				return (10 * src[0] + 8 * (src[-1] + src[1]) - 2 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1] + src[-2] + src[2]) + src[-w2] + src[w2]) >> 4;
			}


			//       -2
			//   -2  +8 -2
			// 1    +10     1		Divisor of 16
			//   -2  +8 -2
			//       -2
			template <typename T> static inline int BayerPhi(T *src, const int w, const int w2)
			{
				return (10 * src[0] + 8 * (src[-w] + src[w]) - 2 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1] + src[-w2] + src[w2]) + src[-2] + src[2]) >> 4;
			}


			static inline void Clamp(int value, uint16_t*, uint16_t *dst, const uint16_t)	{ *dst = value > 65535 ? 65535 : value < 0 ? 0 : value; }
			static inline void Clamp(int value, uint16_t*, byte *dst, const uint16_t shift)	{ value = value >> shift; *dst = value > 255 ? 255 : value < 0 ? 0 : value; }
			static inline void Clamp(int value, byte*, uint16_t *dst, const uint16_t)		{ *dst = value < 0 ? 0 : value; }
			static inline void Clamp(int value, byte*, byte *dst, const uint16_t)			{ *dst = value > 255 ? 255 : value < 0 ? 0 : value; }


			// Even row
			template <typename T, typename U> static inline void BayerEven(T *src, U *dst, const int dw, const int dh, const int sw, const uint16_t shift, bool even)
			{
				int x, y;
				const int row	= dw * 3;
				const int w2	= sw * 2;

				if (even)
				{
					// Even column
					for (y=0; y<dh; y+=2, src+=sw+4, dst+=row)
					{
						for (x=0; x<dw; x+=2)
						{
							Clamp(*src, src, dst++, shift);							// R even
							Clamp(BayerCross(src, sw, w2), src, dst++, shift);		// G even
							Clamp(BayerChecker(src, sw, w2), src, dst++, shift);	// B even
							src++;
							Clamp(BayerTheta(src, sw, w2), src, dst++, shift);		// R odd
							Clamp(*src, src, dst++, shift);							// G odd
							Clamp(BayerPhi(src, sw, w2), src, dst++, shift);		// B odd
							src++;
						}
					}
				}
				else
				{
					// Odd column
					for (y=0; y<dh; y+=2, src+=sw+4, dst+=row)
					{
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerTheta(src, sw, w2), src, dst++, shift);		// R odd
							Clamp(*src, src, dst++, shift);							// G odd
							Clamp(BayerPhi(src, sw, w2), src, dst++, shift);		// B odd
							src++;
							Clamp(*src, src, dst++, shift);							// R even
							Clamp(BayerCross(src, sw, w2), src, dst++, shift);		// G even
							Clamp(BayerChecker(src, sw, w2), src, dst++, shift);	// B even
							src++;
						}
					}
				}
			}


			// Odd row
			template <typename T, typename U> static inline void BayerOdd(T *src, U *dst, const int dw, const int dh, const int sw, const uint16_t shift, bool even)
			{
				int x, y;
				const int row	= dw * 3;
				const int w2	= sw * 2;

				if (even)
				{
					// Even column
					for (y=0; y<dh; y+=2, src+=sw+4, dst+=row)
					{
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerPhi(src, sw, w2), src, dst++, shift);		// R even
							Clamp(*src, src, dst++, shift);							// G even
							Clamp(BayerTheta(src, sw, w2), src, dst++, shift);		// B even
							src++;
							Clamp(BayerChecker(src, sw, w2), src, dst++, shift);	// R odd
							Clamp(BayerCross(src, sw, w2), src, dst++, shift);		// G odd
							Clamp(*src, src, dst++, shift);							// B odd
							src++;
						}
					}
				}
				else
				{
					// Even column
					for (y=0; y<dh; y+=2, src+=sw+4, dst+=row)
					{
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerChecker(src, sw, w2), src, dst++, shift);	// R odd
							Clamp(BayerCross(src, sw, w2), src, dst++, shift);		// G odd
							Clamp(*src, src, dst++, shift);							// B odd
							src++;
							Clamp(BayerPhi(src, sw, w2), src, dst++, shift);		// R even
							Clamp(*src, src, dst++, shift);							// G even
							Clamp(BayerTheta(src, sw, w2), src, dst++, shift);		// B even
							src++;
						}
					}
				}
			}


			// Decode data from a bayer sensor to an RGB image
			// Bayer mode offsets:
			//		0: RG,GB
			//		1: GB,RG
			//		2: GR,BG
			//		3: BG,GR
			template <typename T, typename U> static void BayerColour(T *src, U *dst, int width, int height, byte bayerMode, uint16_t shift)
			{
				// If there are not an even number of rows and columns then do not convert
				if (width % 2 || height % 2) return;

				int dw	= width - 4;
				int dh	= height - 4;
				src 	+= width + width + 2;

				std::future<void> even, odd;

				switch (bayerMode)
				{
					case 0: even	= std::async(std::launch::async, [=]() { BayerEven(src, dst, dw, dh, width, shift, true); });
							odd		= std::async(std::launch::async, [=]() { BayerOdd(src + width, dst + 3 * dw, dw, dh, width, shift, true); });
							break;

					case 1: even	= std::async(std::launch::async, [=]() { BayerOdd(src, dst, dw, dh, width, shift, true); });
							odd		= std::async(std::launch::async, [=]() { BayerEven(src + width, dst + 3 * dw, dw, dh, width, shift, true); });
							break;

					case 2: even	= std::async(std::launch::async, [=]() { BayerEven(src, dst, dw, dh, width, shift, false); });
							odd		= std::async(std::launch::async, [=]() { BayerOdd(src + width, dst + 3 * dw, dw, dh, width, shift, false); });
							break;

					case 3: even	= std::async(std::launch::async, [=]() { BayerOdd(src, dst, dw, dh, width, shift, false); });
							odd		= std::async(std::launch::async, [=]() { BayerEven(src + width, dst + 3 * dw, dw, dh, width, shift, false); });
							break;
				}
			}



			// Greyscale bayer green centre
			template <typename T> static inline int BayerGreyG(T *src, const int w, const int w2)
			{
				return (36 * src[0] + 8 * (src[-1] + src[1] + src[-w] + src[w]) - 4 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1]) - src[-w2] - src[w2] - src[-2] - src[2]) / 48;
			}


			// Greyscale bayer non-green centre
			template <typename T> static inline int BayerGreyN(T *src, const int w, const int w2)
			{
				return (36 * src[0] + 4 * (src[-w-1] + src[-w] + src[-w+1] + src[-1] + src[1] + src[w-1] + src[w] + src[w+1]) - 5 * (src[-w2] + src[w2] + src[-2] + src[2])) / 48;
			}


			// Decode data from a bayer sensor to an greyscale image
			// Bayer mode offsets (see above)
			template <typename T, typename U> static void BayerGrey(T *src, U *dst, int width, int height, byte bayerMode, uint16_t shift)
			{
				// If there are not an even number of rows and columns then do not convert
				if (width % 2 || height % 2) return;

				int x, y;
				int w2	= width * 2;
				int dw	= width - 4;
				int dh	= height - 4;
				src 	+= width + width + 2;

				if (bayerMode == 0 || bayerMode == 3)
				{
					for (y=0; y<dh; y+=2)
					{
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerGreyN(src, width, w2), src, dst++, shift);	src++;
							Clamp(BayerGreyG(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerGreyG(src, width, w2), src, dst++, shift);	src++;
							Clamp(BayerGreyN(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
					}
				}
				else
				{
					for (y=0; y<dh; y+=2)
					{
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerGreyG(src, width, w2), src, dst++, shift);	src++;
							Clamp(BayerGreyN(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
						for (x=0; x<dw; x+=2)
						{
							Clamp(BayerGreyN(src, width, w2), src, dst++, shift);	src++;
							Clamp(BayerGreyG(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
					}
				}
			}

	};
}
