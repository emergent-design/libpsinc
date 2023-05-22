#include "psinc/Instrument.h"
#include "psinc/driver/Commands.h"

#include <emergent/logger/Logger.hpp>
#include <future>

using std::string;
using namespace std::chrono_literals;


namespace psinc
{
	const std::set<uint16_t> Instrument::Vendors::All = { 0x2dd8, 0x0525 };
	const std::set<uint16_t> Instrument::Vendors::PSI = { 0x2dd8 };


	Instrument::~Instrument()
	{
		this->Dispose();
	}


	void Instrument::Dispose()
	{
		if (this->initialised && this->run)
		{
			this->run			= false;
			this->initialised	= false;

			// Notify the thread to wake so that it can then exit
			this->condition.notify_one();
			this->_thread.join();
		}
	}


	Device Instrument::CustomDevice(byte index)
	{
		return { &this->transport, "", index };
	}


	std::map<string, Transport::Info> Instrument::List(uint16_t product, const std::set<uint16_t> &vendors)
	{
		return Transport::List(vendors, product);
	}


	void Instrument::Initialise(uint16_t product, string serial, std::function<void(bool)> onConnection, int timeout, const std::set<uint16_t> &vendors)
	{
		this->onConnection = onConnection;

		// Common devices for all instrument types
		this->devices = {
			{ "Error",				{ &transport, "Error", 				0x04, Device::Direction::Input }},	// Error reporting
			{ "Serial",				{ &transport, "Serial", 			0x05, Device::Direction::Both }},	// Serial number of the camera (16 bytes)
			{ "Storage0",			{ &transport, "Storage0", 			0x06, Device::Direction::Both }},	// Storage block 0 (free for use - 502 bytes)
			{ "Name",				{ &transport, "Name", 				0x07, Device::Direction::Both }},	// User-settable name of the camera.
			{ "Storage1",			{ &transport, "Storage1", 			0x08, Device::Direction::Both }},	// Storage block 1 (free for use - 127 bytes)
			{ "Watchdog",			{ &transport, "Watchdog",			0x0d, Device::Direction::Both }},	// Watchdog time configuration (ms)
			{ "HardwareVersion",	{ &transport, "HardwareVersion",	0x0b, Device::Direction::Input }},
			{ "FirmwareVersion",	{ &transport, "FirmwareVersion",	0x0f, Device::Direction::Input }},
		};

		if (product == Type::Camera)
		{
			this->devices.insert({
				{ "Prox",		{ &this->transport, "Prox", 		0x00, Device::Direction::Input }},	// Prox reader device
				{ "Lock",		{ &this->transport, "Lock",			0x01, Device::Direction::Output }},	// Electronic lock control
				{ "LEDArray",	{ &this->transport, "LEDArray",		0x02, Device::Direction::Output }},	// LED array
				{ "SecureLock",	{ &this->transport, "SecureLock",	0x03, Device::Direction::Both }},	// Encrypted lock control
				{ "Defaults",	{ &this->transport, "Defaults", 	0x09, Device::Direction::Both }},	// Default settings for this device. Modify with care.
				{ "LEDPair",	{ &this->transport, "LEDPair",		0x0e, Device::Direction::Output }},	// Simple LED pair
				{ "Count",		{ &this->transport, "Count",		0x13, Device::Direction::Input }},	// 64-bit counter
				{ "Query",		{ &this->transport, "Query", 		0xff, Device::Direction::Input }}	// Query the camera for a list of available devices and chip type
			});
		}

		if (product == Type::Odometer)
		{
			this->devices.insert({
				{ "Count", { &this->transport, "Count", 0x00, Device::Direction::Input }}		// The odometer count value
			});
		}

		this->transport.Initialise(vendors, (uint16_t)product, serial, [&](bool connected) {
			this->configured = false;

			if (this->onConnection && !connected)
			{
				this->onConnection(false);
			}

		}, timeout);

		if (!this->initialised)
		{
			this->run			= true;
			this->_thread		= std::thread(&Instrument::Entry, this);
			this->initialised	= true;
		}
	}


	void Instrument::Entry()
	{
		// if (pthread_setschedprio(this->_thread.native_handle(), 20)) //sched_get_priority_max(SCHED_FIFO)))
		// 	{
		// 		emg::Log::Warning("Failed to set thread priority for transport layer: %s", strerror(errno));
		// 	}

		// int policy;
		// sched_param param;
		// param.sched_priority = 80; //sched_get_priority_max(SCHED_FIFO);

		// if (pthread_setschedparam(this->_thread.native_handle(), SCHED_FIFO, &param))
		// {
		// 	emg::Log::Warning("Failed to set thread priority for transport layer: %s", strerror(errno));
		// }

		// // std::cout << "max: " << sched_get_priority_max(SCHED_OTHER) << '\n';

		// if (pthread_getschedparam(this->_thread.native_handle(), &policy, &param))
		// {
		// 	std::cout << "error getting schedule params: " << errno << '\n';
		// }

		// std::cout << "priority set to: " << param.sched_priority << " and policy = " << policy << '\n';


		std::unique_lock lock(this->cs);

		while (this->run)
		{
			this->transport.Poll(0);

			if (this->transport.Connected() && !this->configured)
			{
				this->configured = this->Configure();

				if (this->onConnection && this->configured)
				{
					this->onConnection(true);
				}
			}

			// if (this->Main())
			// {
			// 	this->condition.wait_for(lock, 50ms);
			// }
			this->condition.wait_for(lock, this->Main() ? 50ms : 10us);
		}
	}


	bool Instrument::Connected() const
	{
		return this->transport.Connected() && this->configured;
	}


	bool Instrument::Reset(ResetLevel level)
	{
		switch (level)
		{
			case ResetLevel::Connection:	return this->transport.Reset(false);
			case ResetLevel::Control:		return this->transport.Reset(true);
			default:						break;
		}

		std::atomic<bool> waiting(false);
		std::vector<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,						// Header
			Commands::ResetChip, (byte)level, 0x00, 0x00, 0x00,	// Command
			0xff												// Terminator
		};

		if (level == ResetLevel::Imaging || level == ResetLevel::ImagingSoft)
		{
			// Do not allow image grabbing during this reset operation
			std::lock_guard lock(this->cs);

			if (this->transport.Transfer(&command, nullptr, waiting))
			{
				// Give the imaging chip a chance to recover
				std::this_thread::sleep_for(500ms);

				// If just the imaging chip is being reset then the a camera will need
				// to refresh registers, since this is at instrument level then a full
				// reconfiguration is required.
				return this->configured = this->Configure();
			}

			return false;
		}

		return this->transport.Transfer(&command, nullptr, waiting);
	}


	void Instrument::SetTimeout(int timeout)
	{
		this->transport.SetTimeout(timeout);
	}


	uint8_t Instrument::UsbVersion() const
	{
		return this->transport.UsbVersion();
	}
}
