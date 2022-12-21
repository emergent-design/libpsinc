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
			bool Process(bool monochrome, const bool hdr, const std::vector<emg::byte> &data, const int width, const int height, const emg::byte bayerMode) override
			{
				return true;
			}
	};
}
