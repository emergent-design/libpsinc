#include "psinc/driver/Device.h"
#include "psinc/driver/Commands.h"

#include <emergent/logger/Logger.hpp>

using namespace std;
using namespace pugi;
using namespace emergent;

#define READ_BUFFER 512


namespace psinc
{
	Device::Device()
	{
	}


	Device::Device(Transport *transport, string name, byte index, Direction direction)
	{
		this->transport = transport;
		this->index		= index;
		this->name		= name;
		this->direction	= direction;
	}


	string Device::Name()
	{
		return this->name;
	}


	bool Device::Initialise(byte configuration)
	{
		atomic<bool> waiting(false);
		Buffer<byte> data = {
			0x00, 0x00, 0x00, 0x00, 0x00, 										// Header
			Commands::InitialiseDevice, this->index, configuration, 0x00, 0x00,	// Command
			0xff																// Terminator
		};

		return this->transport ? this->transport->Transfer(&data, nullptr, waiting) : false;
	}


	bool Device::Write(const std::string &text)
	{
		return this->Write(reinterpret_cast<const byte *>(text.data()), text.size());
	}


	bool Device::Write(const Buffer<byte> &buffer)
	{
		return this->Write(buffer.Data(), buffer.Size());
	}


	bool Device::Write(const byte *buffer, int size)
	{
		if (size <= 0) return false;

		atomic<bool> waiting(false);
		byte command[]	= {
			0x00, 0x00, 0x00, 0x00, 0x00, 		// Header
			Commands::WriteBlock, this->index, 	// Command
			(byte)(size & 0xff), 				// Size
			(byte)((size >> 8) & 0xff),
			(byte)((size >> 16) & 0xff),
		};

		Buffer<byte> data(11 + size);

		memcpy(data.Data(), command, 10);
		memcpy(data.Data() + 10, data, size);
		data[size+10] = 0xff;					// Terminator

		return this->transport ? this->transport->Transfer(&data, nullptr, waiting) : false;
	}


	bool Device::Write(byte value)
	{
		atomic<bool> waiting(false);
		Buffer<byte> data = {
			0x00, 0x00, 0x00, 0x00, 0x00, 							// Header
			Commands::WriteDevice, this->index, value, 0x00, 0x00,	// Command
			0xff													// Terminator
		};

		return this->transport ? this->transport->Transfer(&data, nullptr, waiting) : false;
	}


	Buffer<byte> Device::Read()
	{
		if (this->direction != Direction::Output)
		{
			atomic<bool> waiting(false);
			Buffer<byte> receive(READ_BUFFER);
			receive = 0;
			Buffer<byte> command = {
				0x00, 0x00, 0x00, 0x00, 0x00,							// Header
				0x03, 0x00, 0x00, 0x00, 0x00, 							// Flush command
				Commands::QueueDevice, this->index, 0x00, 0x00, 0x00, 	// Command
				0xff 													// Terminator
			};

			if (this->transport && this->transport->Transfer(&command, &receive, waiting, false))
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


	bool Device::Read(emg::Buffer<byte> &buffer)
	{
		if (this->direction != Direction::Output)
		{
			atomic<bool> waiting(false);

			buffer		= 0;
			int size	= buffer.Size();

			Buffer<byte> command = {
				0x00, 0x00, 0x00, 0x00, 0x00, 	// Header
				Commands::ReadBlock, 			// Command
				this->index,
				(byte)(size & 0xff),
				(byte)((size >> 8) & 0xff),
				(byte)((size >> 16) & 0xff),
				0xff
			};

			return this->transport && this->transport->Transfer(&command, &buffer, waiting, false, true);
		}

		return false;
	}


	Device::Channel Device::SetChannel(uint16_t channel)
	{
		return this->ManageChannel(0x01, channel & 0xff, (channel >> 8) & 0xff);
	}


	Device::Channel Device::GetChannel()
	{
		return this->ManageChannel(0x00, 0, 0);
	}


	Device::Channel Device::ManageChannel(byte mode, byte low, byte high)
	{
		atomic<bool> waiting(false);
		Buffer<byte> receive = { 0, 0, 0, 0 };
		Buffer<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,						// Header
			Commands::Channel, this->index, mode, low, high,	// Command
			0xff
		};

		if (transport && this->transport->Transfer(&command, &receive, waiting))
		{
			return {
				(uint16_t)((receive[1] << 8) + receive[0]),	// Channel ID
				(uint16_t)((receive[3] << 8) + receive[2])	// Type ID
			};
		}

		return { 0, 0 };
	}


	Device::Information Device::Query()
	{
		Information result;
		atomic<bool> waiting(false);
		Buffer<byte> receive(13);
		Buffer<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,					// Header
			Commands::Query, this->index, 0x00, 0x00, 0x00,	// Command
			0xff
		};

		receive = 0;

		if (transport && this->transport->Transfer(&command, &receive, waiting))
		{
			result.index			= receive[0];
			result.functionality	= (receive[2] << 8) + receive[1];
			result.channels			= (receive[4] << 8) + receive[3];

			for (int i=0; i<4; i++)
			{
				result.reserved[i]	= receive[5 + i];
				result.status[i]	= receive[9 + i];
			}
		}

		return result;
	}

}

