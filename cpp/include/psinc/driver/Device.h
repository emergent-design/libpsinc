#pragma once

#include <emergent/Emergent.h>
#include <emergent/xml/pugixml.hpp>
#include <psinc/transport/Transport.h>
#include <map>
#include <memory>


namespace psinc
{
	class Device
	{
		public:
			enum class Direction
			{
				Input,
				Output,
				Both
			};
			
			Device();
			Device(Transport *transport, std::string name, byte index, Direction direction = Direction::Both);
			
			Direction GetDirection(std::string direction);
			
			std::string Name();
			
			
			bool Initialise(byte configuration);
			bool Write(std::string text);
			bool Write(emg::Buffer<byte> &buffer);
			bool Write(byte value);
			
			emg::Buffer<byte> Read();
			
			// Can this be removed since Buffer<byte> can be implicitly cast
			// to a string?
			std::string ReadString();
			
		protected:
			std::string name;
			byte index				= 0;
			Direction direction		= Direction::Both;
			Transport *transport	= nullptr;
	};

	/// Definition for the map of features owned by the camera
	//typedef std::map<std::string, std::unique_ptr<Device>> DeviceMap;
}
