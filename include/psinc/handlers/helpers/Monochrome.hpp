#pragma once

#include <emergent/Maths.hpp>


namespace psinc
{
	using emg::byte;


	// Decode data from a monochrome sensor
	class Monochrome
	{
		public:

			template <typename T, typename U> static void Decode(T *src, U *dst, int width, int height, int depth, uint16_t shift)
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
	};
}
