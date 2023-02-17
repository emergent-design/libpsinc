#pragma once

#include <emergent/Timer.hpp>

#include <emergent/Emergent.hpp>
#include <emergent/parallel/Generator.hpp>
#include <execution>
#include <limits>
#include <cmath>


namespace psinc::bayer
{
	using emg::byte;


	template <typename T, typename U> class Demosaic
	{
		public:

			// Decode data from a bayer sensor to an RGB image. It is a simpler algorithm than many of the modern options such as RCD, VNG, HPHD
			// but since this is required for a continuous stream of images rather than a single RAW photo, performance was a primary concern.
			// Therefore it produces somewhat noisier output in regions of solid colour than other methods. It does, however, attempt to limit
			// some of the artefacts found with the original bayer decoder, such as zippering around coloured edges, by using gradient-based weighting
			// when determining the missing green values. The red and blue channel interpolation uses a very simple local neighbourhood colour differential
			// algorithm which assumes that chromaticity changes at a much lower frequency than intensity.
			static bool Decode(const byte bayerMode, const T *src, const size_t width, const size_t height, const byte depth, U *dst, const size_t shift)
			{
				if constexpr (std::is_same_v<T, U>)
				{
					if (depth == 3)
					{
						return Decode(CFA[bayerMode], src, width, height, dst);
					}
				}

				thread_local std::vector<T> buffer;
				buffer.resize(width * height * 3);

				Decode(CFA[bayerMode], src, width, height, buffer.data());

				if (depth == 1)
				{
					return Greyscale(buffer.data(), width * height, dst, shift);
				}

				std::transform(std::execution::par_unseq, buffer.begin(), buffer.end(), dst, [&](const auto &v) {
					if constexpr (sizeof(U) < sizeof(T))
					{
						return std::clamp<T>(v >> shift, 0, std::numeric_limits<U>::max());
					}
					return v;
				});

				return true;
			}


		private:

			static constexpr float EPSILON 							= 1e-6;
			static constexpr std::array<const byte[2][2], 4> CFA	= {{
				{{ 0, 1 }, { 1, 2 }},	// 0: RG,GB
				{{ 1, 2 }, { 0, 1 }},	// 1: GB,RG
				{{ 1, 0 }, { 2, 1 }},	// 2: GR,BG
				{{ 2, 1 }, { 1, 0 }}	// 3: BG,GR
			}};


			static inline byte channel(const byte cfa[2][2], const size_t x, const size_t y)
			{
				return cfa[y & 1][x & 1];
			}


			static bool Greyscale(const T *src, const size_t size, U *dst, const size_t shift)
			{
				const emg::Generator<size_t> generator(0, size, 1);

				std::for_each(
					std::execution::par_unseq, generator.begin(), generator.end(),
					[&](const size_t i) {
						const T *ps		= src + i * 3;
						const T value	= (ps[0] + ps[1] + ps[2]) / 3;

						if constexpr (sizeof(U) < sizeof(T))
						{
							dst[i] = std::clamp<T>(value >> shift, 0, std::numeric_limits<U>::max());
						}
						else
						{
							dst[i] = value;
						}
					}
				);

				return true;
			}


			static bool Decode(const byte cfa[2][2], const T *src, const size_t width, const size_t height, T *dst)
			{
				// std::array<uint64_t, 5> times = { 0 };
				// emg::Timer timer;

				const emg::Generator<size_t> generator(0, height, 1);

				// Populate the channels that are known
				std::for_each(
					std::execution::par_unseq, generator.begin(), generator.end(),
					[&](const size_t y) {
						const T *ps = src + y * width;
						T *pd		= dst + y * width * 3;

						for (size_t x=0; x<width; x++, ps++, pd+=3)
						{
							pd[channel(cfa, x, y)] = ps[0];
						}
					}
				);

				// times[0] = timer.MicroElapsed();	timer.Reset();

				PopulateGreen(cfa, src, width, height, dst);		// times[1] = timer.MicroElapsed(); timer.Reset();
				PopulateRedBlue(cfa, dst, width, height);			// times[2] = timer.MicroElapsed(); timer.Reset();
				PopulateRedBlueAtGreen(cfa, dst, width, height);	// times[3] = timer.MicroElapsed(); timer.Reset();
				InterpolateBorder(cfa, 3, dst, width, height);		// times[4] = timer.MicroElapsed(); timer.Reset();

				// for (size_t i=0; i<times.size(); i++)
				// {
				// 	std::printf("%zu : %lu\n", i, times[i]);
				// }
				// std::printf("\n");

				return true;
			}



			// Calculate the gradients at the RB positions and use this to populate the green values
			// Using a 5x5 matrix incorporates the differentials in all 3 colour channels
			static void PopulateGreen(const byte cfa[2][2], const T *src, const size_t width, const size_t height, T *dst)
			{
				const emg::Generator<size_t> generator(2, height - 4, 1);
				const int w1 = width * 1;
				const int w2 = width * 2;

				std::for_each(
					std::execution::par_unseq, generator.begin(), generator.end(),
					[&](const size_t y) {
						// Start the row on a non-green pixel
						const size_t sx = 2 + (channel(cfa, 0, y) & 1);
						auto *ps		= src + y * width + sx;
						T *pd			= dst + y * width * 3 + sx * 3;

						for (size_t x=sx; x<width-2; x+=2, ps+=2, pd+=6)
						{
							// Calculate the intensity gradients between like colour channels in each direction
							//
							// Example layout when on the red pixel
							//   R00 G01 R02 G03 R04
							//   G10 B11 G12 B13 G14
							//   R20 G21 R22 G23 R24
							//   G30 B31 G32 B33 G34
							//   R40 G41 R42 G43 R44
							//
							// Vertical gradient   = | 0.5 * (R02 + R42) - R22 | + | G12 - G32 |
							// Horizontal gradient = | 0.5 * (R20 + R24) - R22 | + | G21 - G23 |
							const float gradV = EPSILON + std::abs(0.5f * (ps[-w2] + ps[w2]) - ps[0]) + std::abs(ps[-w1] - ps[w1]);
							const float gradH = EPSILON + std::abs(0.5f * (ps[ -2] + ps[ 2]) - ps[0]) + std::abs(ps[-1] - ps[1]);

							// Based on the original -1 2 4 2 -1 filter but with a directional gradient bias
							const float value = 0.5f * ps[0] + 0.5f * (
								  gradV * (ps[-1] + ps[1] - 0.5f * (ps[ -2] + ps[ 2]))		// strength of vertical edge contributes power to horizontal values
								+ gradH * (ps[-w1] + ps[w1] - 0.5f * (ps[-w2] + ps[w2]))	// strength of horizontal edge contributes power to vertical values
							) / (gradV + gradH);

							if constexpr (std::is_floating_point_v<T>)
							{
								pd[1] = value;
							}
							else
							{
								pd[1] = std::clamp<int>(std::lrint(value), 0, std::numeric_limits<T>::max());
							}
						}
					}
				);
			}


			// Populate red and blue at blue and red positions
			static void PopulateRedBlue(const byte cfa[2][2], T *dst, const size_t width, const size_t height)
			{
				const emg::Generator<size_t> generator(2, height - 4, 1);
				// Offsets in 3-channel space
				const int nw = -width * 3 - 3;
				const int ne = -width * 3 + 3;
				const int sw =  width * 3 - 3;
				const int se =  width * 3 + 3;

				std::for_each(
					std::execution::par_unseq, generator.begin(), generator.end(),
					[&](const size_t y) {
						// Start the row on a non-green pixel
						const size_t sx = 2 + (channel(cfa, 0, y) & 1);
						const size_t ch = 2 - channel(cfa, sx, y);		// blue if on red / red if on blue
						T *pd			= dst + y * width * 3 + sx * 3;

						for (size_t x=sx; x<width-2; x+=2, pd+=6)
						{
							// mean colour differential of the surrounding pixels for the channel we are interpolating
							// compared with the previously interpolated green value at the same location
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
				);
			}


			// Populate red and blue at green positions
			// Possibly use the neighbourhood gradients to direct this?
			static void PopulateRedBlueAtGreen(const byte cfa[2][2], T *dst, const size_t width, const size_t height)
			{
				const emg::Generator<size_t> generator(2, height - 4, 1);
				// Offsets in 3-channel space
				const int n = -width * 3;
				const int s =  width * 3;
				const int e = -3;
				const int w =  3;

				std::for_each(
					std::execution::par_unseq, generator.begin(), generator.end(),
					[&](const size_t y) {
						// Start the row on a green pixel
						const size_t sx = 2 + (channel(cfa, 1, y) & 1);
						T *pd			= dst + y * width * 3 + sx * 3;

						for (size_t x=sx; x<width-2; x+=2, pd+=6)
						{
							for (size_t ch : { 0, 2 }) // red and blue
							{
								// mean colour differential of the surrounding pixels for the channel we are interpolating
								// compared with the previously interpolated green value at the same location
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
				);
			}


			static inline void CalculateMeans(const byte cfa[2][2], T *pd, const int x, const int y, const int width, const int height)
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
			static void InterpolateBorder(const byte cfa[2][2], const size_t border, T *dst, const size_t width, const size_t height)
			{
				const size_t edge = width - border;
				const emg::Generator<size_t> generator(0, height, 1);

				std::for_each(
					std::execution::par_unseq, generator.begin(), generator.end(),
					[&](const size_t y) {
						T *pd = dst + y * width * 3;

						if (y < border || y >= height - border)
						{
							// Top and bottom
							for (size_t x=0; x<width; x++, pd+=3)
							{
								CalculateMeans(cfa, pd, x, y, width, height);
							}
						}
						else
						{
							// Left border
							for (size_t x=0; x<border; x++)
							{
								CalculateMeans(cfa, pd + x * 3, x, y, width, height);
							}

							// Right border
							for (size_t x=edge; x<width; x++)
							{
								CalculateMeans(cfa, pd + x * 3, x, y, width, height);
							}
						}
					}
				);
			}
	};
}
