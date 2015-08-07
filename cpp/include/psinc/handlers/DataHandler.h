#pragma once

#include <emergent/Emergent.hpp>
#include <emergent/struct/Buffer.hpp>
#include <atomic>
#include <map>


namespace psinc
{
	/// Base class for handling the data returned from a capture device.
	class DataHandler
	{
		public:

			virtual ~DataHandler() {}

			/// Process the data in the supplied buffer using the known width and height of the image
			virtual bool Process(bool monochrome, emg::Buffer<byte> &data, int width, int height) = 0;

			// This should only be written to by the acquisition system, but can be read
			// from outside to know when the USB transport has prepped the camera for
			// an image grab - only really useful when the camera is acting as a slave.
			std::atomic<bool> waiting;

		protected:

			/// Helper to decode data from a monochrome sensor
			void DecodeMono(byte *src, byte *dst, int width, int height, int depth);

			/// Helper to decode data from  a bayer sensor to a greyscale image
			void DecodeGrey(byte *src, byte *dst, int width, int height);

			/// Helper to decode data from a bayer sensor to an RGB image
			void DecodeColour(byte *src, byte *dst, int width, int height);
	};
}
