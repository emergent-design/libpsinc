#include "psinc/driver/Register.h"
#include "psinc/driver/Commands.h"

using namespace pugi;
using namespace emergent;


namespace psinc
{
	Register::Register(xml_node configuration, Transport *transport)
	{
		this->address	= strtol(configuration.attribute("address").as_string("0x00"), nullptr, 0);
		this->offset	= 2 * (address & 0xff);
		this->page		= (address >> 8) & 0xff;
		this->value		= 0;
		this->transport	= transport;
	}


	void Register::Initialise(int offset, int mask, int value)
	{
		this->value = (this->value & ~mask) | ((value << offset) & mask);
	}


	byte Register::Page()
	{
		return this->page;
	}


	int Register::Get()
	{
		return this->value;
	}


	int Register::Address()
	{
		return this->address;
	}


	void Register::Set(int offset, int mask, int value)
	{
		bool waiting	= false;
		this->value 	= (this->value & ~mask) | ((value << offset) & mask);
		
		Buffer<byte> data = { 
			0x00, 0x00, 0x00, 0x00, 0x00, 			// Header
			Commands::WriteRegister, 				// Command
			(byte)(this->address & 0xff),
			(byte)((this->address >> 8) & 0xff),
			(byte)(this->value & 0xff),
			(byte)((this->value >> 8) & 0xff),
			0xff									// Terminator
		};

		this->transport->Transfer(&data, nullptr, waiting);
	}

	
	void Register::SetBit(int offset, bool value)
	{
		bool waiting = false;
		if (value) 	this->value |= 1 << offset;
		else 		this->value &= ~(1 << offset);
		
		Buffer<byte> data = { 
			0x00, 0x00, 0x00, 0x00, 0x00, 
			Commands::WriteBit, (byte)(this->address & 0xff), (byte)((this->address >> 8) & 0xff), (byte)offset, (byte)((value ? 1 : 0)),
			0xff
		};


		this->transport->Transfer(&data, nullptr, waiting);
	}
	
	
	bool Register::Refresh()
	{
		bool waiting = false;
		Buffer<byte> receive(5);
		Buffer<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,																				// Header 
			0x03, 0x00, 0x00, 0x00, 0x00, 																				// Flush command
			Commands::QueueRegister, (byte)(this->address & 0xff), (byte)((this->address >> 8) & 0xff), 0x00, 0x00, 	// Command
			0xff 																										// Terminator
		};

		if (this->transport->Transfer(&command, &receive, waiting))
		{
			this->value = (receive[3] << 8) + receive[4];
			
			return true;
		}
		
		return false;
	}


	void Register::Refresh(Buffer<byte> &data)
	{
		if (this->offset < data.Size() - 1)
		{
			this->value = (data[this->offset] << 8) + data[this->offset + 1];
		}
	}
}
