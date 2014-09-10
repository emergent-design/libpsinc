#pragma once

#define PUGIXML_NO_XPATH
#include <emergent/Emergent.h>
#include <emergent/xml/pugixml.hpp>
#include <psinc/transport/Transport.h>
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


			/// Constructors
			Device();
			Device(Transport *transport, std::string name, byte index, Direction direction = Direction::Both);


			/// Returns the name of this device
			std::string Name();


			/// Send an initialisation byte to the device
			bool Initialise(byte configuration);


			/// Write a string to the device
			bool Write(std::string text);


			/// Write the binary data to the device
			bool Write(emg::Buffer<byte> &buffer);


			/// Write a single byte to the device
			bool Write(byte value);


			/// Read binary data from the device
			emg::Buffer<byte> Read();


		protected:

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
