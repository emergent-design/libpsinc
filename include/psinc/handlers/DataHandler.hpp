#pragma once

#include <emergent/Emergent.hpp>
#include <emergent/struct/Buffer.hpp>
#include <atomic>
// #include <map>


namespace psinc
{
	/// Base class for handling the data returned from a capture device.
	class DataHandler
	{
		public:

			virtual ~DataHandler() {}

			/// Process the data in the supplied buffer using the known width and height of the image
			virtual bool Process(bool monochrome, const bool hdr, const std::vector<emg::byte> &data, const size_t width, const size_t height, const emg::byte bayerMode) = 0;

			// This should only be written to by the acquisition system, but can be read
			// from outside to know when the USB transport has prepped the camera for
			// an image grab - only really useful when the camera is acting as a slave.
			std::atomic<bool> waiting;
	};
}
