#pragma once

#include <emergent/Emergent.h>
#include <emergent/xml/pugixml.hpp>
#include "psinc/transport/Transport.h"


namespace psinc
{
	/// Representation of a device memory register (can contain values for multiple features).
	class Register
	{
		public:
		
			Register() {}
			
			Register(pugi::xml_node configuration, Transport *transport);
		
			int Get();
			
			// Like setting but without triggering messaging
			void Initialise(int offset, int mask, int value);
			
			void Set(int offset, int mask, int value);
			void SetBit(int offset, bool value);
			
			bool Refresh();
			void Refresh(emg::Buffer<byte> &data);
			
			int Address();
			byte Page();
			
		private:
			int value;
			int offset;
			int address;
			byte page;
			Transport *transport;
	};
}
