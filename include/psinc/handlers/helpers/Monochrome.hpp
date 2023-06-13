#pragma once

#include <emergent/Maths.hpp>
#include <cstring>

// #if __has_include(<execution>)
// 	#include <emergent/parallel/Generator.hpp>
// 	#include <execution>
// #endif

namespace psinc
{
	using emg::byte;


	// Decode data from a monochrome sensor
	class Monochrome
	{
		public:

			// #if __has_include(<execution>)
			// 	template <typename T, typename U> static bool Decode(const T *src, U *dst, const size_t width, const size_t height, const byte depth, const uint16_t shift)
			// 	{
			// 		const emg::Generator<size_t> generator(0, width * height, 1);

			// 		if (depth == 3)
			// 		{
			// 			std::for_each(
			// 				std::execution::par_unseq, generator.begin(), generator.end(),
			// 				[&](const size_t i) {
			// 					U *pd = dst + i * 3;

			// 					if constexpr (sizeof(U) < sizeof(T))
			// 					{
			// 						pd[0] = pd[1] = pd[2] = std::clamp<T>(src[i] >> shift, 0, std::numeric_limits<U>::max());
			// 					}
			// 					else
			// 					{
			// 						pd[0] = pd[1] = pd[2] = src[i];
			// 					}
			// 				}
			// 			);
			// 		}
			// 		else if (std::is_same_v<T, U>)
			// 		{
			// 			std::memcpy(dst, src, generator.count * sizeof(T));
			// 		}
			// 		else
			// 		{
			// 			std::for_each(
			// 				std::execution::par_unseq, generator.begin(), generator.end(),
			// 				[&](const size_t i)
			// 				{
			// 					if constexpr (sizeof(U) < sizeof(T))
			// 					{
			// 						dst[i] = std::clamp<T>(src[i] >> shift, 0, std::numeric_limits<U>::max());
			// 					}
			// 					else
			// 					{
			// 						dst[i] = src[i];
			// 					}
			// 				}
			// 			);
			// 		}

			// 		return true;
			// 	}

			// #else

				template <typename T, typename U> static bool Decode(const T *src, U *dst, const size_t width, const size_t height, const byte depth, const uint16_t shift)
				{
					using emg::Maths;

					const size_t size = width * height;

					if (depth == 3)
					{
						for (size_t i=0; i<size; i++, dst+=3)
						{
							if constexpr (sizeof(U) < sizeof(T))
							{
								dst[0] = dst[1] = dst[2] = std::clamp<T>(*src++ >> shift, 0, std::numeric_limits<U>::max());
							}
							else
							{
								dst[0] = dst[1] = dst[2] = *src++;
							}
						}
					}
					else if (std::is_same_v<T, U>)
					{
						std::memcpy(dst, src, size * sizeof(T));
					}
					else
					{
						for (size_t i=0; i<size; i++)
						{
							if constexpr (sizeof(U) < sizeof(T))
							{
								// *dst++ = Maths::clamp<U>(*src++ >> shift);
								*dst++ = std::clamp<T>(*src++ >> shift, 0, std::numeric_limits<U>::max());
							}
							else
							{
								*dst++ = *src++;
							}
						}
					}

					return true;
				}

			// #endif
	};
}
