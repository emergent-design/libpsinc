#include "psinc/Instrument.h"
#include "psinc/driver/Commands.h"

#include <emergent/logger/Logger.hpp>


using namespace std;
using namespace emergent;


namespace psinc
{

	Instrument::Instrument()
	{
		this->deviceSets[Type::Camera] = map<byte, Device> {
			{ 0x00, { &this->transport, "Prox", 		0x00, Device::Direction::Input }},	// Prox reader device
			{ 0x01, { &this->transport, "Lock",			0x01, Device::Direction::Output }},	// Electronic lock control
			{ 0x02, { &this->transport, "LEDArray",		0x02, Device::Direction::Output }},	// LED array
			{ 0x03, { &this->transport, "SecureLock",	0x03 }},							// Encrypted lock control
			{ 0x09, { &this->transport, "Defaults", 	0x09 }},							// Default settings for this device. Modify with care.
			{ 0x0e, { &this->transport, "LEDPair",		0x0e, Device::Direction::Output }},	// Simple LED pair
			{ 0x13, { &this->transport, "Count",		0x13, Device::Direction::Input }},	// 64-bit counter
			{ 0xff, { &this->transport, "Query", 		0xff, Device::Direction::Input }}	// Query the camera for a list of available devices and chip type
		};

		this->deviceSets[Type::Odometer] = map<byte, Device> {
			{ 0x00, { &this->transport, "Count", 		0x00, Device::Direction::Input }},	// The odometer count value
		};

		// Notes:
		// The value stored in the "Name" device also appears as part of the serial in the USB descriptor for this instrument.
		for (auto &s : this->deviceSets)
		{
			s.second[0x04] = { &this->transport, "Error", 		0x04, Device::Direction::Input };	// Error reporting
			s.second[0x05] = { &this->transport, "Serial", 		0x05 };								// Serial number of the camera (16 bytes)
			s.second[0x06] = { &this->transport, "Storage0", 	0x06 };								// Storage block 0 (free for use - 502 bytes)
			s.second[0x07] = { &this->transport, "Name", 		0x07, Device::Direction::Both };	// User-settable name of the camera.
			s.second[0x08] = { &this->transport, "Storage1", 	0x08 };								// Storage block 1 (free for use - 127 bytes)
		}
	}


	Instrument::~Instrument()
	{
		if (this->initialised)
		{
			this->run = false;
			this->_thread.join();
		}
	}


	Device Instrument::CustomDevice(byte index)
	{
		return { &this->transport, "", index };
	}


	vector<string> Instrument::List(Type product)
	{
		return TransportHotplug::List((int)product);
	}


	void Instrument::Initialise(Type product, string serial, std::function<void(bool)> onConnection, int timeout)
	{
		this->transport.Initialise((int)product, serial, onConnection, timeout);

		this->devices.clear();
		for (auto &d : this->deviceSets[product]) this->devices[d.second.Name()] = d.second;

		if (!this->initialised)
		{
			this->run			= true;
			this->_thread		= thread(&Instrument::Entry, this);
			this->initialised	= true;
		}
	}


	void Instrument::Entry()
	{
		while (this->run)
		{
			this->transport.Poll(0);
			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}


	bool Instrument::Connected()
	{
		return this->transport.Connected();
	}


	bool Instrument::Reset()
	{
		return this->transport.Reset();
	}
}


