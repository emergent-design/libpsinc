#pragma once

#include <psinc/handlers/DataHandler.h>


namespace psinc
{
	/// An image specific data handler. This provides the conversion from mono/bayer
	/// formatted data in the buffer to a greyscale or colour image of the required
	/// type.
	class ImageHandler : public DataHandler
	{
		public:

			ImageHandler(bool forceBayer = false) : forceBayer(forceBayer) {}

			/// Constructor
			/// @param[in] image Reference to an image buffer that will be populated with the converted data.
			ImageHandler(emg::ImageBase<byte> &image, bool forceBayer = false);


			void Initialise(emg::ImageBase<byte> &image, bool forceBayer = false);


			// Refer to parent documentation
			virtual bool Process(bool monochrome, emg::Buffer<byte> &data, int width, int height);

		protected:

			/// The image to be populated.
			emg::ImageBase<byte> *image = nullptr;
			bool forceBayer;
	};
}
