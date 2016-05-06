#pragma once

#include <psinc/handlers/DataHandler.h>
#include <psinc/handlers/Decoder.hpp>
#include <emergent/image/Image.hpp>



namespace psinc
{
	/// An image specific data handler. This provides the conversion from mono/bayer
	/// formatted data in the buffer to a greyscale or colour image of the required
	/// type.
	template <typename T> class ImageHandler : public DataHandler
	{
		public:

			ImageHandler() {}


			ImageHandler(emg::ImageBase<T> &image, bool forceBayer = false)
			{
				this->Initialise(image, forceBayer);
			}


			void Initialise(emg::ImageBase<T> &image, bool forceBayer = false)
			{
				this->image			= &image;
				this->forceBayer	= forceBayer;
			}


			// Set the amount of bitshifting (right) to perform when dealing with HDR data
			// stored to byte image.
			void Shift(uint16_t bits)
			{
				this->shiftBits = bits;
			}


			virtual bool Process(bool monochrome, bool hdr, emg::Buffer<byte> &data, int width, int height, byte bayerMode)
			{
				// Allow the monochrome flag to be overridden - could have unexpected effects.
				if (this->forceBayer) monochrome = false;

				int w = monochrome ? width : width - 4;
				int h = monochrome ? height : height - 4;

				if (w > 0 && h > 0 && data.Size() == width * height * (hdr ? 2 : 1))
				{
					int depth = this->image->Depth();
					this->image->Resize(w, h);

					if (hdr)
					{
						if (monochrome)			Decoder::Mono((uint16_t *)data.Data(), this->image->Data(), width, height, depth, this->shiftBits);
						else if (depth == 3)	Decoder::BayerColour((uint16_t *)data.Data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
						else					Decoder::BayerGrey((uint16_t *)data.Data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
					}
					else
					{
						if (monochrome)			Decoder::Mono(data.Data(), this->image->Data(), width, height, depth, this->shiftBits);
						else if (depth == 3)	Decoder::BayerColour(data.Data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
						else					Decoder::BayerGrey(data.Data(), this->image->Data(), width, height, bayerMode, this->shiftBits);
					}

					return true;
				}

				return false;
			}

		protected:

			emg::ImageBase<T> *image	= nullptr;
			bool forceBayer				= false;


			// If dealing with HDR data, then apply this bit shift to incoming data
			// before converting from ushort to byte. The default is 8 for using the
			// most significant byte. This is only applied if the destination image is byte.
			uint16_t shiftBits = 8;
	};
}

