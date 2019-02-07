#pragma once

#include <emergent/Maths.hpp>
#include <emergent/image/Image.hpp>


namespace psinc
{
	class Filter
	{
		public:

			enum Mode
			{
				Disabled,
				RowOffset,	// Adjust brightness of specific rows by a fixed offset
				RowGain		// Adjust brightness of specific rows by a gain multiplier
			};

			struct Configuration
			{
				Mode mode	= Mode::Disabled;
				int start	= 0;				// Starting row of the mark-space pattern
				int mark	= 2;				// Number of rows to apply the offset/gain to
				int space	= 5;				// Number of rows to then skip
				int offset	= 0;				// Offset to be applied to pixel values in mark rows
				double gain	= 1.0;				// Gain multiplier to be applied to pixel values in mark rows
			};


			template <typename T> static void Process(const Configuration &configuration, emg::ImageBase<T> &image)
			{
				switch (configuration.mode)
				{
					case Mode::Disabled:	break;
					case Mode::RowOffset:	Apply(configuration, image, [=](const T s) { return s + configuration.offset; });			break;
					case Mode::RowGain:		Apply(configuration, image,	[=](const T s) { return std::lrint(configuration.gain * s); });	break;
				}
			}


		private:


			template <typename T, typename Operation> static void Apply(const Configuration &configuration, emg::ImageBase<T> &image, Operation operation)
			{
				int x, y;
				const int width		= image.Width();
				const int height	= image.Height();
				const int depth		= image.Depth();
				const int row		= width * depth;
				const int mark		= configuration.mark * row;
				const int space		= configuration.space * row;
				const int limit		= height - configuration.mark;
				const int window	= configuration.mark + configuration.space;
				T *src				= image + configuration.start * row;

				for (y=configuration.start; y<height; y+=window, src+=space)
				{
					const int end = y < limit ? mark : (height - y) * row;

					for (x=0; x<end; x++, src++)
					{
						*src = emg::Maths::clamp<T>(operation(*src));
					}
				}
			}
	};
}
