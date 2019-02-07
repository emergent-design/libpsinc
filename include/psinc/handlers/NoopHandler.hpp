#pragma once

#include <psinc/handlers/DataHandler.hpp>


namespace psinc
{
	/// A no-op data handler to allow frames to be captured but not processed in
	/// any way. Useful when throwing a frame away after context switching.
	class NoopHandler : public DataHandler
	{
		public:

			// Refer to parent documentation
			bool Process(bool monochrome, bool hdr, emg::Buffer<emg::byte> &data, int width, int height, emg::byte bayerMode) override
			{
				return true;
			}
	};
}
