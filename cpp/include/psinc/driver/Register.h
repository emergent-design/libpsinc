#pragma once

#include <emergent/Emergent.hpp>
#include <emergent/xml/pugixml.hpp>
#include "psinc/transport/Transport.h"


namespace psinc
{
	/// Representation of a camera memory register (can contain values for multiple features).
	class Register
	{
		public:

			Register() {}
			Register(pugi::xml_node configuration, Transport *transport);


			/// Get the value of this register
			int Get();


			/// Sets the value but without triggering messaging
			void Initialise(int offset, int mask, int value);


			/// Sets the value of this register and transmits to the camera
			void Set(int offset, int mask, int value);


			/// Sets an individual bit of this register and transmits to the camera
			void SetBit(int offset, bool value);


			/// Refresh the local value of this register by reading from the camera
			bool Refresh();


			/// When performing a full refresh of all registers, the values are returned
			/// in page blocks and this is used to update a particular register from that
			/// data block.
			void Refresh(emg::Buffer<byte> &data);


			/// Returns the address of this particular register
			int Address();


			/// Returns the page that this register belongs to
			byte Page();

		private:

			/// The current local value of this register
			int value;

			/// The offset of this register within a page block
			int offset;

			/// The address of this register
			int address;

			/// The page that this register belongs to
			byte page;

			/// Reference to the transport layer (owned by the camera)
			Transport *transport;
	};
}
