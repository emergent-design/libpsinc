#pragma once

#define PUGIXML_NO_XPATH
#include <emergent/Emergent.hpp>
#include <emergent/xml/pugixml.hpp>
#include <psinc/Transport.h>
#include <map>
#include <memory>


namespace psinc
{
	/// Represents a device that is either a capability of the camera or
	/// hardware that can be controlled by the camera.
	class Device
	{
		public:

			/// Indicates the direction of communication with this device
			enum class Direction
			{
				Input,	///< Values can be read from this device
				Output,	///< Values can be written to this device
				Both	///< Values can be both written to and read from this device
			};

			struct Channel
			{
				uint16_t id;
				uint16_t type;
			};

			struct Information
			{
				byte index				= 0;	///< Which device this is
				uint16_t functionality	= 0;	///< Bitmask indicating which device functions are supported
				uint16_t channels		= 0;	///< Number of available channels (contiguous, zero-indexed)
				byte reserved[4];				///> Reserved registers for future use
				byte status[4];					///> Status flag registers
			};


			/// Constructors
			Device();
			Device(Transport *transport, std::string name, byte index, Direction direction = Direction::Both);


			/// Returns the name of this device
			std::string Name();


			/// Send an initialisation byte to the device
			bool Initialise(byte configuration);


			/// Write a string to the device
			bool Write(const std::string &text);


			/// Write the binary data to the device
			bool Write(const emg::Buffer<byte> &buffer);


			/// Write a single byte to the device
			bool Write(byte value);


			/// Read binary data from the device
			emg::Buffer<byte> Read();


			/// Read binary data from the device into the supplied buffer. The buffer size
			/// is used to determine how much data can be received from the device
			bool Read(emg::Buffer<byte> &buffer);


			/// Set the channel for this device. Will return the channel information including
			/// the type so that the protocol for communication can be determined.
			Channel SetChannel(uint16_t channel);

			/// Will return the channel information including the type so that the protocol for
			/// communication can be determined.
			Channel GetChannel();

			/// Query this device providing details such as number of channels available and
			/// status flags.
			Information Query();


		protected:

			Channel ManageChannel(byte mode, byte low, byte high);

			/// The name of this device
			std::string name;

			/// The device index
			byte index = 0;

			/// The device direction
			Direction direction	= Direction::Both;

			/// Reference to the transport layer (owned by the camera)
			Transport *transport = nullptr;
	};
}
