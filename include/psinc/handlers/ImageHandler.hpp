#pragma once

#include <psinc/handlers/DataHandler.hpp>
#include <psinc/handlers/helpers/Bayer.hpp>
#include <psinc/handlers/helpers/Monochrome.hpp>
// #include <psinc/handlers/helpers/Filter.hpp>
#include <emergent/image/Image.hpp>


namespace psinc
{
	enum class DecodeMode
	{
		Automatic	= 0,	// Automatically determine whether or not to use bayer decoding
		Invert		= 1,	// Invert the sensor type - so treat bayer as mono and vice versa
		ForceBayer	= 2,	// Force the sensor to be treated as bayer
		ForceMono	= 3		// Force the sensor to be treated as mono
	};

	/// An image specific data handler. This provides the conversion from mono/bayer
	/// formatted data in the buffer to a greyscale or colour image of the required
	/// type.
	template <typename T> class ImageHandler : public DataHandler
	{
		public:

			struct Configuration
			{
				DecodeMode mode = DecodeMode::Automatic;
			};

			ImageHandler() {}


			ImageHandler(emg::ImageBase<T> &image, const Configuration &configuration = {})
			{
				this->Initialise(image, configuration);
			}


			void Initialise(emg::ImageBase<T> &image, const Configuration &configuration = {})
			{
				this->image			= &image;
				this->configuration	= configuration;
			}

			void Initialise(emg::ImageBase<T> &image, bool invertSensorType)
			{
				this->image					= &image;
				this->configuration.mode	= invertSensorType ? DecodeMode::Invert : DecodeMode::Automatic;
			}


			// Set the amount of bitshifting (right) to perform when dealing with HDR data
			// stored to byte image.
			void Shift(uint16_t bits)
			{
				this->shiftBits = bits;
			}


			bool Process(bool monochrome, const bool hdr, const std::vector<byte> &data, const int width, const int height, const byte bayerMode) override
			{
				// Allow the monochrome flag to be overridden - could have unexpected effects.
				switch (configuration.mode)
				{
					case DecodeMode::Invert: 		monochrome = !monochrome;	break;
					case DecodeMode::ForceBayer:	monochrome = false;			break;
					case DecodeMode::ForceMono:		monochrome = true;			break;
					default:													break;
				}

				const int w = monochrome ? width : width - 4;
				const int h = monochrome ? height : height - 4;

				if (w > 0 && h > 0 && data.size() == width * height * (hdr ? 2 : 1))
				{
					const byte depth = this->image->Depth();
					this->image->Resize(w, h);

					if (hdr)
					{
						if (monochrome)			Monochrome::Decode((uint16_t *)data.data(), this->image->Data(), width, height, depth, this->shiftBits);
						else if (depth == 3)	Bayer::Colour((uint16_t *)data.data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
						else					Bayer::Grey((uint16_t *)data.data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
					}
					else
					{
						if (monochrome)			Monochrome::Decode(data.data(), this->image->Data(), width, height, depth, this->shiftBits);
						else if (depth == 3)	Bayer::Colour(data.data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
						else					Bayer::Grey(data.data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
					}

					// Filter::Process(this->configuration.filter, *this->image);

					return true;
				}

				return false;
			}

		protected:

			emg::ImageBase<T> *image = nullptr;
			Configuration configuration;


			// If dealing with HDR data, then apply this bit shift to incoming data
			// before converting from ushort to byte. The default is 8 for using the
			// most significant byte. This is only applied if the destination image is byte.
			uint16_t shiftBits = 8;
	};
}

