#pragma once
#include <emergent/image/Image.hpp>

// #include <iostream>
#include <emergent/Timer.hpp>

namespace psinc::bayer
{
	using emg::byte;


	class Demosaic
	{
		public:
				// Decode data from a bayer sensor to an RGB image
			// Bayer mode offsets:
			//		0: RG,GB
			//		1: GB,RG
			//		2: GR,BG
			//		3: BG,GR

			// R00 G01 R02 G03 R04
			// G10 B11 G12 B13 G14
			// R20 G21 R22 G23 R24
			// G30 B31 G32 B33 G34
			// R40 G41 R42 G43 R44

			// G00 B01 G02 B03 G04
			// R10 G11 R12 G13 R14
			// G20 B21 G22 B23 G24
			// R30 G31 R32 G33 R34
			// G40 B41 G42 B43 G44

			// G00 R01 G02 R03 G04
			// B10 G11 B12 G13 B14
			// G20 R21 G22 R23 G24
			// B30 G31 B32 G33 B34
			// G40 R41 G42 R43 G44

			// B00 G01 B02 G03 B04
			// G10 R11 G12 R13 G14
			// B20 G21 B22 G23 B24
			// G30 R31 G32 R33 G34
			// B40 G41 B42 G43 B44



			template <typename T> bool Decode(const byte cfa[2][2], const std::vector<T> &src, const size_t width, const size_t height, emg::ImageBase<T> &dst)
			{
				dst.Resize(width, height, 3);

				std::array<uint64_t, 5> times = { 0 };
				emg::Timer timer;

				auto *ps	= src.data();
				T *pd		= dst.Data();

				// std::printf("%d %d\n", width, height);
				// float *pr 	= raw.data();
					// float *pb	= buffer;

				// this->debug.Resize(width, height);
				// auto *pdbg	= debug.Data();

				// Populate the channels that are known
				for (size_t y=0; y<height; y++)
				{
					// std::printf("%d %d\n", y, height);

					for (size_t x=0; x<width; x++, ps++, pd+=3) //, pdbg++)
					{
						// std::printf("%d ", (int)channel(cfa, x, y));
						// std::printf("%ld %ld %d %d %d %d\n", (int64_t)ps, (int64_t)pd, width, height, x, y);

						// if (channel(cfa, x, y))
						// {
						// 	pdbg[0] = ps[0] / (float)std::numeric_limits<T>::max();
						// }
						pd[channel(cfa, x, y)] = ps[0]; // / (float)std::numeric_limits<T>::max();
					}
				}

				times[0] = timer.MicroElapsed();	timer.Reset();

				this->PopulateGreen(cfa, src, width, height, dst);		times[1] = timer.MicroElapsed(); timer.Reset();
				this->PopulateRedBlue(cfa, dst, width, height);			times[2] = timer.MicroElapsed(); timer.Reset();
				this->PopulateRedBlueAtGreen(cfa, dst, width, height);	times[3] = timer.MicroElapsed(); timer.Reset();
				this->InterpolateBorder(cfa, 3, dst, width, height);	times[4] = timer.MicroElapsed(); timer.Reset();

				// this->debug.Save("green-debug.tiff");

				for (size_t i=0; i<5; i++)
				{
					std::printf("%d : %d\n", i, times[i]);
				}
				std::printf("\n");

				return true;
			}


		private:

			static constexpr float EPSILON = 1e-6;

			static inline byte channel(const byte cfa[2][2], const size_t x, const size_t y)
			{
				return cfa[y & 1][x & 1];
			}




			// Calculate the gradients at the RB positions and use this to populate the green values
			// Using a 5x5 matrix incorporates the differentials in all 3 colour channels
			template <typename T> void PopulateGreen(const byte cfa[2][2], const std::vector<T> &src, const size_t width, const size_t height, emg::ImageBase<T> &dst)
			{
				// Use a double for summing floating point values and a 64-bit int for summing all integer types
				// using Sum = std::conditional_t<std::is_floating_point_v<T>, double, int64_t>;

				// this->gradients.Resize(width, height);

				// this->gradients.Resize(width, height);
				const int w1 = width * 1;
				const int w2 = width * 2;

				for (size_t y=2; y<height-2; y++)
				{
					// Start the row on a non-green pixel
					const size_t sx = 2 + (channel(cfa, 0, y) & 1);

					auto *ps	= src.data() + y * width + sx;
					T *pd		= dst.Data() + y * width * 3 + sx * 3;
					// float *pg	= gradients.Data() + y * width + sx;
					// auto *pdbg	= debug.Data() + y * width + sx;

					for (size_t x=sx; x<width-2; x+=2, ps+=2, pd+=6) //, pg+=2) //, pdbg+=2)
					{
						const float n = ps[-w1];
						const float s = ps[w1];
						const float e = ps[1];
						const float w = ps[-1];

						// Calculate the maximum difference between like colour channels in each direction
						// const float gradV = EPSILON + 0.5f * (std::abs(ps[w1] - ps[-w1]) + std::abs(ps[w2] - ps[-w2]));
						// const float gradH = EPSILON + 0.5f * (std::abs(ps[1] - ps[-1]) + std::abs(ps[2] - ps[-2]));

						// const float gradV = EPSILON + std::abs(0.5f * (ps[-w2] + ps[w2]) - ps[0]);
						// const float gradH = EPSILON + std::abs(0.5f * (ps[-2] + ps[2]) - ps[0]);

						const float gradV = EPSILON + std::abs(0.5f * (ps[-w2] + ps[w2]) - ps[0]) + 0.5f * std::abs(n - s);
						const float gradH = EPSILON + std::abs(0.5f * (ps[ -2] + ps[ 2]) - ps[0]) + 0.5f * std::abs(w - e);

						// const float gradB = EPSILON + 0.5f * (std::abs(ps[w1+1] - ps[-w1-1]) + std::abs(ps[w2+2] - ps[-w2-2]));	// backslash diagonal
						// const float gradF = EPSILON + 0.5f * (std::abs(ps[w1-1] - ps[-w1+1]) + std::abs(ps[w2-2] - ps[-w2+2]));	// forward slash
						// const float gradD = EPSILON + 0.25f * ()

						//const float gradV = EPSILON + std::abs(s - n);
						//const float gradH = EPSILON + std::abs(e - w);
						// const float gradB = EPSILON + std::abs(0.5f * (s+e) - 0.5f * (n+w));	// backslash diagonal
						// const float gradF = EPSILON + std::abs(0.5f * (n+e) - 0.5f * (s+w));	// forward slash

						// const float gradNW = EPSILON + 0.05 * (std::abs(ps[0] - ps[-w2-2])); // + std::abs(e - ps[-w2-1]) + std::abs(s - ps[-w1-2]));
						// const float gradNE = EPSILON + 0.05 * (std::abs(ps[0] - ps[-w2+2])); // + std::abs(w - ps[-w2+1]) + std::abs(s - ps[-w1+2]));
						// const float gradSW = EPSILON + 0.05 * (std::abs(ps[0] - ps[ w2-2])); // + std::abs(e - ps[ w2-1]) + std::abs(n - ps[ w1-2]));
						// const float gradSE = EPSILON + 0.05 * (std::abs(ps[0] - ps[ w2+2])); // + std::abs(w - ps[ w2+1]) + std::abs(n - ps[ w1+2]));


						// const float sum = gradV + gradH + gradB;

						// if (x >= 674 && x <= 676 && y >= 427 && y <= 429)
						// {
						// 	// std::printf("%.1f %.1f %.1f %.1f\n", gradV, gradH, gradB, gradF);
						// 	std::printf("%.1f %.1f %.1f %.1f %.1f %.1f\n", gradV, gradH, gradNW, gradNE, gradSW, gradSE);
						// }

						// const float scale = 1.f / (gradV + gradH + gradB + gradF);
						// const float scale = 1.f / (gradV + gradH + gradNW + gradNE + gradSW + gradSE);
						const float scale = 1.f / (gradV + gradH);

						// const float h = 0.5f * (w + e);
						// const float v = 0.5f * (n + s);
						// const float a = 0.5f * (h + v);


						// Based on the original -1 2 4 2 -1 filter but with a directional gradient bias
						const float value = 0.5f * ps[0] + 0.5f * scale * (
							  gradV * (w + e - 0.5f * (ps[ -2] + ps[ 2]))							// strength of vertical edge contributes power to horizontal G values
							+ gradH * (n + s - 0.5f * (ps[-w2] + ps[w2]))							// strength of horizontal edge contributes power to vertical G values
						);

						// const float value = (std::abs(gradV - gradH) < EPSILON)
						// 	? 0.25f * (n + s + e + w)
						// 	: 0.5f * ps[0] + 0.5f * scale * (
						// 	  gradV * (w + e - 0.5f * (ps[ -2] + ps[ 2]))							// strength of vertical edge contributes power to horizontal G values
						// 	+ gradH * (n + s - 0.5f * (ps[-w2] + ps[w2]))							// strength of horizontal edge contributes power to vertical G values
						// );

						// const float value = gradV > gradH
						// 	? 0.666f * ps[0] + 0.333f * (w + e) - 0.166f * (ps[ -2] + ps[ 2])
						// 	: 0.666f * ps[0] + 0.333f * (n + s) - 0.166f * (ps[-w2] + ps[w2]);


						// const float value = 0.5f * ps[0]
						// 	+ 0.25f * ps[-w1]
						// 	+ 0.25f * ps[ w1]
						// 	+ 0.25f * ps[ -1]
						// 	+ 0.25f * ps[  1]
						// 	- 0.0125f * ps[-w2]
						// 	- 0.0125f * ps[ w2]
						// 	- 0.0125f * ps[ -2]
						// 	- 0.0125f * ps[  2];


						// const float value = scale * (
						// 	  gradV * h							// strength of vertical edge contributes power to horizontal G values
						// 	+ gradH * v							// strength of horizontal edge contributes power to vertical G values
						// 	// + scale * (gradB + gradF) * 0.5f * (h + v);	// rest of the power is shared using both horizontal and vertical G pixels
						// 	+ gradB * a //0.125f * (a + ps[-w1+2] + ps[-w2+1] + ps[w2-1] + ps[w1-2])
						// 	+ gradF * a //0.125f * (a + ps[-w1-2] + ps[-w2-1] + ps[w2+1] + ps[w1+2])
						// );

						// pg[0] = scale * gradV;

						if constexpr (std::is_floating_point_v<T>)
						{
							pd[1] = value;
						}
						else
						{
							pd[1] = std::clamp<int>(std::lrint(value), 0, std::numeric_limits<T>::max());
						}

						// pd[1] = std::clamp<Sum>(value, 0, std::numeric_limits<T>::max());
						// pdbg[0] = value / (float)std::numeric_limits<T>::max();


						// const int64_t gradR
					}
				}

				// this->gradients.Save("cardinal-gradients.tiff");

			}


			// Populate red and blue at blue and red positions
			template <typename T> void PopulateRedBlue(const byte cfa[2][2], emg::ImageBase<T> &dst, const size_t width, const size_t height)
			{
				// Offsets in 3-channel space
				const int nw = -width * 3 - 3;
				const int ne = -width * 3 + 3;
				const int sw =  width * 3 - 3;
				const int se =  width * 3 + 3;

				for (size_t y=2; y<height-2; y++)
				{
					// Start the row on a non-green pixel
					const size_t sx = 2 + (channel(cfa, 0, y) & 1);
					const size_t ch = 2 - channel(cfa, sx, y);		// blue if on red / red if on blue
					T *pd			= dst.Data() + y * width * 3 + sx * 3;

					for (size_t x=sx; x<width-2; x+=2, pd+=6)
					{
						// mean colour differential of the surrounding pixels of the channel we are interpolating
						const float diff = 0.25f * (
							  (pd[nw + ch] - pd[nw + 1])
							+ (pd[ne + ch] - pd[ne + 1])
							+ (pd[sw + ch] - pd[sw + 1])
							+ (pd[se + ch] - pd[se + 1])
						);

						if constexpr (std::is_floating_point_v<T>)
						{
							pd[ch] = pd[1] + diff;
						}
						else
						{
							pd[ch] = std::clamp<int>(std::lrint(pd[1] + diff), 0, std::numeric_limits<T>::max());
						}

					}
				}
			}


			// Populate red and blue at green positions
			// Possibly use the neighbourhood gradients to direct this?
			template <typename T> void PopulateRedBlueAtGreen(const byte cfa[2][2], emg::ImageBase<T> &dst, const size_t width, const size_t height)
			{
				// Offsets in 3-channel space
				const int n = -width * 3;
				const int s =  width * 3;
				const int e = -3;
				const int w =  3;

				for (size_t y=2; y<height-2; y++)
				{
					// Start the row on a green pixel
					const size_t sx = 2 + (channel(cfa, 1, y) & 1);
					T *pd			= dst.Data() + y * width * 3 + sx * 3;

					for (size_t x=sx; x<width-2; x+=2, pd+=6)
					{
						for (size_t ch : { 0, 2 })
						{
							// mean colour differential of the surrounding pixels of the channel we are interpolating
							const float diff = 0.25f * (
								  (pd[n + ch] - pd[n + 1])
								+ (pd[e + ch] - pd[e + 1])
								+ (pd[s + ch] - pd[s + 1])
								+ (pd[w + ch] - pd[w + 1])
							);

							if constexpr (std::is_floating_point_v<T>)
							{
								pd[ch] = pd[1] + diff;
							}
							else
							{
								pd[ch] = std::clamp<int>(std::lrint(pd[1] + diff), 0, std::numeric_limits<T>::max());
							}
						}
					}
				}
			}


			template <typename T> static inline void CalculateMeans(const byte cfa[2][2], T *pd, const int x, const int y, const int width, const int height)
			{
				using Sum = std::conditional_t<std::is_floating_point_v<T>, double, int64_t>;

				Sum sum[3]		= { 0 };
				Sum tally[3]	= { 0 };
				const int sy	= y > 0 ? -1 : 0;
				const int ey	= y < height-1 ? 2 : 1;
				const int sx	= x > 0 ? -1 : 0;
				const int ex	= x < width - 1 ? 2 : 1;
				const int row	= width * 3;

				for (int dy=sy; dy<ey; dy++)
				{
					for (int dx=sx; dx<ex; dx++)
					{
						const byte ch = channel(cfa, x+dx, y+dy);
						sum[ch] += pd[dy * row + dx * 3 + ch];
						tally[ch]++;
					}
				}

				const byte ch = channel(cfa, x, y);

				for (byte c=0; c<3; c++)
				{
					pd[c] = c == ch ? pd[c] : sum[c] / tally[c];
				}
			}


			// Border must be greater than zero otherwise this might be unhappy
			template <typename T> void InterpolateBorder(const byte cfa[2][2], const size_t border, emg::ImageBase<T> &dst, const size_t width, const size_t height)
			{
				const size_t edge = width - border;

				// Outer columns
				for (size_t y=0; y<height; y++)
				{
					T *pd = dst.Data() + y * width * 3;

					for (size_t x=0; x<border; x++)
					{
						CalculateMeans(cfa, pd + x * 3, x, y, width, height);
					}

					for (size_t x=edge; x<width; x++)
					{
						CalculateMeans(cfa, pd + x * 3, x, y, width, height);
					}
				}

				// Outer rows
				for (size_t y=0; y<border; y++)
				{
					T *pd = dst.Data() + y * width * 3 + border * 3;

					for (size_t x=border; x<edge; x++, pd+=3)
					{
						CalculateMeans(cfa, pd, x, y, width, height);
					}
				}

				for (size_t y=height - border; y<height; y++)
				{
					T *pd = dst.Data() + y * width * 3 + border * 3;

					for (size_t x=border; x<edge; x++, pd+=3)
					{
						CalculateMeans(cfa, pd, x, y, width, height);
					}
				}
			}


			// emg::Image<float> gradients;
	};
}
