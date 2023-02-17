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
			bool Process(bool, const bool, const std::vector<emg::byte> &, const size_t, const size_t, const emg::byte) override
			{
				return true;
			}
	};
}
