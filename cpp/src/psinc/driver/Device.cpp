#include "psinc/driver/Device.h"
#include "psinc/driver/Commands.h"

#include <emergent/Logger.h>

using namespace std;
using namespace pugi;
using namespace emergent;

#define READ_BUFFER 512


namespace psinc
{
	Device::Device()
	{
		// Need a default constructor because the device map will automatically create a new instance
		// of device if accessed with an unknown key. Should probably respond to that here somehow! Throw
		// an exception perhaps?
		LOG(error, "A device was created without configuration, probably due to accessing an invalid key in the device map!");
	}


	Device::Device(Transport *transport, string name, byte index, Direction direction)
	{
		this->transport = transport;
		this->index		= index;
		this->name		= name;
		this->direction	= direction;
	}


	Device::Direction Device::GetDirection(string direction)
	{
		if (direction == "input")	return Direction::Input;
		if (direction == "output")	return Direction::Output;
		
		return Direction::Both;
	}


	string Device::Name()
	{
		return this->name;
	}


	bool Device::Initialise(byte configuration)
	{
		bool waiting = false;
		Buffer<byte> data = { 
			0x00, 0x00, 0x00, 0x00, 0x00, 										// Header 
			Commands::InitialiseDevice, this->index, configuration, 0x00, 0x00,	// Command
			0xff																// Terminator
		};
		
		return this->transport->Transfer(&data, nullptr, waiting);
	}
	
	
	bool Device::Write(std::string text)
	{
		return this->Write(Buffer<byte>(text));
	}
		
	
	bool Device::Write(Buffer<byte> &buffer)
	{
		int size		= buffer.Size();
		bool waiting	= false;
		byte command[]	= {
			0x00, 0x00, 0x00, 0x00, 0x00, 		// Header 
			Commands::WriteBlock, this->index, 	// Command
			(byte)(size & 0xff), 				// Size
			(byte)((size >> 8) & 0xff), 
			(byte)((size >> 16) & 0xff),	
		};

		Buffer<byte> data(11 + size);
		
		memcpy(data.Data(), command, 10);
		memcpy(data.Data() + 10, buffer.Data(), size);
		data[size+10] = 0xff;					// Terminator
		
		return this->transport->Transfer(&data, nullptr, waiting);
	}
	
	
	bool Device::Write(byte value)
	{
		bool waiting = false;
		Buffer<byte> data = { 
			0x00, 0x00, 0x00, 0x00, 0x00, 							// Header 
			Commands::WriteDevice, this->index, value, 0x00, 0x00,	// Command
			0xff													// Terminator
		};
		
		return this->transport->Transfer(&data, nullptr, waiting);
	}
	
	
	Buffer<byte> Device::Read()
	{
		if (this->direction != Direction::Output)
		{
			bool waiting = false;
			Buffer<byte> receive(READ_BUFFER);
			Buffer<byte> command = {
				0x00, 0x00, 0x00, 0x00, 0x00,							// Header 
				0x03, 0x00, 0x00, 0x00, 0x00, 							// Flush command
				Commands::QueueDevice, this->index, 0x00, 0x00, 0x00, 	// Command
				0xff 													// Terminator
			};
			
			if (this->transport->Transfer(&command, &receive, waiting, false))
			{
				int length = (receive[3] << 8) + receive[2];
				
				if (receive[0] == 0x00 && receive[1] == this->index && length > 0 && length < READ_BUFFER - 4)
				{
					return Buffer<byte>(receive.Data() + 4, length);
				}
			}
		}
		
		return {};
	}
	
	
	string Device::ReadString()
	{
		auto receive = this->Read();
		
		return receive.Size() ? string((char *)receive.Data(), receive.Size()) : "";
	}
}