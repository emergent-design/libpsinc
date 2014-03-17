#pragma once

#include <emergent/Emergent.h>
#include <emergent/image/Image.h>
#include <map>

namespace psinc
{
	/// Base class for handling the data returned from a capture device.
	class DataHandler
	{
		public:
		
			virtual ~DataHandler() {}
			
			/// Process the data in the supplied buffer. The configuration metric map
			/// also provides information about the nature of the data in the buffer
			/// (for example, the ImageHandler needs to know the width and height of
			/// image data so that it can convert it from bayer format into a standard
			/// emergent Image<>)
			virtual bool Process(bool monochrome, emg::Buffer<byte> &data, int width, int height) = 0;

			// This should only be written to by the acquisition system, but can be read
			// from outside to know when the USB transport has prepped the camera for
			// an image grab - only really useful when the camera is acting as a slave.
			volatile bool waiting = false;
			
		protected:
			
			void DecodeMono(byte *src, byte *dst, int width, int height, int depth);
			void DecodeGrey(byte *src, byte *dst, int width, int height);
			void DecodeColour(byte *src, byte *dst, int width, int height);
	};
}
