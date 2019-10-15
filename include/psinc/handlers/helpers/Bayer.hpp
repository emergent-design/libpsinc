#pragma once

#include <emergent/Maths.hpp>
#include <emergent/thread/Persistent.hpp>


namespace psinc
{
	using emg::byte;

	/// Decode data from a Bayer filtered sensor
	class Bayer
	{
		public:

			//       -1
			//       +2
			// -1 +2 +4 +2 -1		Divisor of 8
			//       +2
			//       -1
			template <typename T> static inline int Cross(T *src, const int w, const int w2)
			{
				return (4 * src[0] + 2 * (src[-w] + src[w] + src[-1] + src[+1])  - src[-w2] - src[w2] - src[-2] - src[+2]) >> 3;
			}


			//        -3
			//    +4     +4
			// -3    +12    -3		Divisor of 16
			//    +4     +4
			//        -3
			template <typename T> static inline int Checker(T *src, const int w, const int w2)
			{
				return (12 * src[0] + 4 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1]) - 3 * (src[-w2] + src[w2] + src[-2] + src[2])) >> 4;
			}


			//         1
			//    -2     -2
			// -2 +8 +10 +8 -2		Divisor of 16
			//    -2     -2
			//         1
			template <typename T> static inline int Theta(T *src, const int w, const int w2)
			{
				return (10 * src[0] + 8 * (src[-1] + src[1]) - 2 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1] + src[-2] + src[2]) + src[-w2] + src[w2]) >> 4;
			}


			//       -2
			//   -2  +8 -2
			// 1    +10     1		Divisor of 16
			//   -2  +8 -2
			//       -2
			template <typename T> static inline int Phi(T *src, const int w, const int w2)
			{
				return (10 * src[0] + 8 * (src[-w] + src[w]) - 2 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1] + src[-w2] + src[w2]) + src[-2] + src[2]) >> 4;
			}


			static inline void Clamp(int value, uint16_t*, uint16_t *dst, const uint16_t)	{ *dst = value > 65535 ? 65535 : value < 0 ? 0 : value; }
			static inline void Clamp(int value, uint16_t*, byte *dst, const uint16_t shift)	{ value = value >> shift; *dst = value > 255 ? 255 : value < 0 ? 0 : value; }
			static inline void Clamp(int value, byte*, uint16_t *dst, const uint16_t)		{ *dst = value < 0 ? 0 : value; }
			static inline void Clamp(int value, byte*, byte *dst, const uint16_t)			{ *dst = value > 255 ? 255 : value < 0 ? 0 : value; }


			// Even row
			template <typename T, typename U> static inline void Even(T *src, U *dst, const int dw, const int dh, const int sw, const uint16_t shift, bool even)
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
							Clamp(*src, src, dst++, shift);					// R even
							Clamp(Cross(src, sw, w2), src, dst++, shift);	// G even
							Clamp(Checker(src, sw, w2), src, dst++, shift);	// B even
							src++;
							Clamp(Theta(src, sw, w2), src, dst++, shift);	// R odd
							Clamp(*src, src, dst++, shift);					// G odd
							Clamp(Phi(src, sw, w2), src, dst++, shift);		// B odd
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
							Clamp(Theta(src, sw, w2), src, dst++, shift);	// R odd
							Clamp(*src, src, dst++, shift);					// G odd
							Clamp(Phi(src, sw, w2), src, dst++, shift);		// B odd
							src++;
							Clamp(*src, src, dst++, shift);					// R even
							Clamp(Cross(src, sw, w2), src, dst++, shift);	// G even
							Clamp(Checker(src, sw, w2), src, dst++, shift);	// B even
							src++;
						}
					}
				}
			}


			// Odd row
			template <typename T, typename U> static inline void Odd(T *src, U *dst, const int dw, const int dh, const int sw, const uint16_t shift, bool even)
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
							Clamp(Phi(src, sw, w2), src, dst++, shift);		// R even
							Clamp(*src, src, dst++, shift);					// G even
							Clamp(Theta(src, sw, w2), src, dst++, shift);	// B even
							src++;
							Clamp(Checker(src, sw, w2), src, dst++, shift);	// R odd
							Clamp(Cross(src, sw, w2), src, dst++, shift);	// G odd
							Clamp(*src, src, dst++, shift);					// B odd
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
							Clamp(Checker(src, sw, w2), src, dst++, shift);	// R odd
							Clamp(Cross(src, sw, w2), src, dst++, shift);	// G odd
							Clamp(*src, src, dst++, shift);					// B odd
							src++;
							Clamp(Phi(src, sw, w2), src, dst++, shift);		// R even
							Clamp(*src, src, dst++, shift);					// G even
							Clamp(Theta(src, sw, w2), src, dst++, shift);	// B even
							src++;
						}
					}
				}
			}


			#if defined(_MSC_VER)
				// The MSVC compiler and runtimes have an issue with joining threads
				// that have been created within a thread_local object. Instead use
				// std::async which is implemented using a threadpool in MSVC.
				#define PSINC_ASYNC(x) std::async(std::launch::async, x)
			#else
				// GCC/Clang/MinGW do not use a threadpool for std::async so
				// use PersistentThread instead
				#define PSINC_ASYNC(x) thread.Run(x)
			#endif


			// Decode data from a bayer sensor to an RGB image
			// Bayer mode offsets:
			//		0: RG,GB
			//		1: GB,RG
			//		2: GR,BG
			//		3: BG,GR
			template <typename T, typename U> static void Colour(T *src, U *dst, int width, int height, byte bayerMode, uint16_t shift)
			{
				#if !defined(_MSC_VER)
					// This function tends to be called repeatedly, so to avoid the overhead of thread construction when using std::async
					// it uses a persistent thread instead. The PersistentThread is used by emergent::ThreadPool but even a ThreadPool of
					// size 1 will create another thread to manage the queue, so using the PersistentThread directly is more efficient.
					static thread_local emg::PersistentThread thread;
				#endif

				// If there are not an even number of rows and columns then do not convert
				if (width % 2 || height % 2) return;

				int dw	= width - 4;
				int dh	= height - 4;
				src 	+= width + width + 2;

				std::future<void> f;

				switch (bayerMode)
				{
					case 0: f = PSINC_ASYNC([=] { Even(src, dst, dw, dh, width, shift, true); });
							Odd(src + width, dst + 3 * dw, dw, dh, width, shift, true);
							break;

					case 1: f = PSINC_ASYNC([=] { Odd(src, dst, dw, dh, width, shift, true); });
							Even(src + width, dst + 3 * dw, dw, dh, width, shift, true);
							break;

					case 2: f = PSINC_ASYNC([=] { Even(src, dst, dw, dh, width, shift, false); });
							Odd(src + width, dst + 3 * dw, dw, dh, width, shift, false);
							break;

					case 3: f = PSINC_ASYNC([=] { Odd(src, dst, dw, dh, width, shift, false); });
							Even(src + width, dst + 3 * dw, dw, dh, width, shift, false);
							break;
				}

				f.wait();
			}


			// Greyscale bayer green centre
			template <typename T> static inline int GreyG(T *src, const int w, const int w2)
			{
				return (36 * src[0] + 8 * (src[-1] + src[1] + src[-w] + src[w]) - 4 * (src[-w-1] + src[-w+1] + src[w-1] + src[w+1]) - src[-w2] - src[w2] - src[-2] - src[2]) / 48;
			}


			// Greyscale bayer non-green centre
			template <typename T> static inline int GreyN(T *src, const int w, const int w2)
			{
				return (36 * src[0] + 4 * (src[-w-1] + src[-w] + src[-w+1] + src[-1] + src[1] + src[w-1] + src[w] + src[w+1]) - 5 * (src[-w2] + src[w2] + src[-2] + src[2])) / 48;
			}


			// Decode data from a bayer sensor to a greyscale image
			// Bayer mode offsets (see above)
			template <typename T, typename U> static void Grey(T *src, U *dst, int width, int height, byte bayerMode, uint16_t shift)
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
							Clamp(GreyN(src, width, w2), src, dst++, shift);	src++;
							Clamp(GreyG(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
						for (x=0; x<dw; x+=2)
						{
							Clamp(GreyG(src, width, w2), src, dst++, shift);	src++;
							Clamp(GreyN(src, width, w2), src, dst++, shift);	src++;
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
							Clamp(GreyG(src, width, w2), src, dst++, shift);	src++;
							Clamp(GreyN(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
						for (x=0; x<dw; x+=2)
						{
							Clamp(GreyN(src, width, w2), src, dst++, shift);	src++;
							Clamp(GreyG(src, width, w2), src, dst++, shift);	src++;
						}
						src += 4;
					}
				}
			}

	};
}
