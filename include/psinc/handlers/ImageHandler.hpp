#pragma once

#include <psinc/handlers/DataHandler.hpp>
#include <psinc/handlers/helpers/Monochrome.hpp>
#include <emergent/image/Image.hpp>

#if __has_include(<execution>)
	#include <psinc/handlers/helpers/Demosaic.hpp>
#else
	#include <psinc/handlers/helpers/Bayer.hpp>
#endif


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


			bool Process(bool monochrome, const bool hdr, const std::vector<byte> &data, const size_t width, const size_t height, const byte bayerMode) override
			{
				if (!this->image)
				{
					return false;
				}

				// Allow the monochrome flag to be overridden - could have unexpected effects.
				switch (configuration.mode)
				{
					case DecodeMode::Invert: 		monochrome = !monochrome;	break;
					case DecodeMode::ForceBayer:	monochrome = false;			break;
					case DecodeMode::ForceMono:		monochrome = true;			break;
					default:													break;
				}

				if (data.size() != width * height * (hdr ? 2 : 1))
				{
					return false;
				}

				if (monochrome)
				{
					this->image->Resize(width, height);

					return hdr
						? Monochrome::Decode((uint16_t *)data.data(), this->image->Data(), width, height, this->image->Depth(), this->shiftBits)
						: Monochrome::Decode(data.data(), this->image->Data(), width, height, this->image->Depth(), this->shiftBits);
				}

				#if __has_include(<execution>)	// newer compilers only

					this->image->Resize(width, height);

					return hdr
						? bayer::Demosaic<uint16_t, T>::Decode(bayerMode, (uint16_t *)data.data(), width, height, image->Depth(), image->Data(), shiftBits)
						: bayer::Demosaic<uint8_t, T>::Decode(bayerMode, data.data(), width, height, image->Depth(), image->Data(), shiftBits);

				#else
					const int w = monochrome ? width : width - 4;
					const int h = monochrome ? height : height - 4;

					this->image->Resize(w, h);

					if (this->image->Depth() == 3)
					{
						return hdr
							? Bayer::Colour((uint16_t *)data.data(), image->Data(), width, height, bayerMode, shiftBits)
							: Bayer::Colour(data.data(), image->Data(), width, height, bayerMode, shiftBits);
					}

					return hdr
						? Bayer::Grey((uint16_t *)data.data(), image->Data(), width, height, bayerMode, shiftBits)
						: Bayer::Grey(data.data(), image->Data(), width, height, bayerMode, shiftBits);
				#endif
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

