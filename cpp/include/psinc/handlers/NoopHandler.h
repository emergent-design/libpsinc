#pragma once

#include <psinc/handlers/DataHandler.h>


namespace psinc
{
	/// A no-op data handler to allow frames to be captured but not processed in
	/// any way. Useful when throwing a frame away after context switching.
	class NoopHandler : public DataHandler
	{
		public:

			// Refer to parent documentation
			virtual bool Process(bool monochrome, emg::Buffer<byte> &data, int width, int height)
			{
				return true;
			}
	};
}
