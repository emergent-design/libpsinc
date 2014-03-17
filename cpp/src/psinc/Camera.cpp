#include "psinc/Camera.h"
#include "psinc/xml/Devices.h"
#include "psinc/driver/Commands.h"

#include <emergent/Logger.h>

#define REFRESH_ATTEMPTS 3

using namespace std;
using namespace pugi;
using namespace emergent;


namespace psinc
{

	Camera::Camera()
	{
		// Notes:
		// The "Name" also appears as part of the serial in the USB descriptor for this device.
		this->devicePool = {
			{ 0x00, { &this->transport, "Prox", 		0x00, Device::Direction::Input }},	// Prox reader device
			{ 0x01, { &this->transport, "Lock",			0x01, Device::Direction::Output }},	// Electronic lock control
			{ 0x02, { &this->transport, "LEDArray",		0x02, Device::Direction::Output }},	// LED array
			{ 0x03, { &this->transport, "SecureLock",	0x03 }},							// Encrypted lock control
			{ 0x04, { &this->transport, "Error", 		0x04, Device::Direction::Input }},	// Error reporting
			{ 0x05, { &this->transport, "Serial", 		0x05 }},							// Serial number of the camera (16 bytes)
			{ 0x06, { &this->transport, "Storage0", 	0x06 }},							// Storage block 0 (free for use - 502 bytes)
			{ 0x07, { &this->transport, "Name", 		0x07, Device::Direction::Both }},	// User-settable name of the camera.
			{ 0x08, { &this->transport, "Storage1", 	0x08 }},							// Storage block 1 (free for use - 127 bytes)
			{ 0x09, { &this->transport, "Defaults", 	0x09 }},							// Default settings for this device. Modify with care.
			{ 0x0e, { &this->transport, "LEDPair",		0x0e, Device::Direction::Output }},	// Simple LED pair
			{ 0xff, { &this->transport, "Query", 		0xff, Device::Direction::Input }}	// Query the camera for a list of available devices and chip type
		};
		
		this->send = Buffer<byte>({ 
			0x00, 0x00, 0x00, 0x00, 0x00, 	// Header 
			0x00, 0x00, 0x00, 0x00, 0x00,	// Command here
			0xff							// Terminator
		});
	}


	Camera::~Camera()
	{
		if (this->initialised)
		{
			this->exit = true;

			// Notify the thread to wake so that it can then exit
			this->condition.notify_one();
			this->_thread.join();
		}
	}

	
	void Camera::Initialise(string serial, int bus)
	{
		this->transport.Initialise(serial, bus);
		
		if (!this->initialised)
		{
			this->_thread		= thread(&Camera::Entry, this);
			this->initialised	= true;
		}
	}


	void Camera::Entry()
	{
		unique_lock<mutex> lock(this->cs);
		bool stream = false;

		while (!this->exit)
		{
			FLOG(debug, "Capture thread %s is active", this_thread::get_id());

			if (this->handler)
			{
				if (this->callback)
				{
					if (this->Connected() || this->Connect())
					{
						stream = this->callback(this->Capture(this->handler, this->mode, this->flash));
					}
					else stream = this->callback(ACQUISITION_DISCONNECTED);
				}

				if (!stream) this->handler = nullptr;
			}
			else
			{
				FLOG(debug, "Capture thread %s is going to sleep", this_thread::get_id());
				this->condition.wait(lock);	// Thread is paused until notified
			}
		}

		FLOG(debug, "Capture thread %s is exiting", this_thread::get_id());
	}


	bool Camera::GrabImage(Mode mode, DataHandler &handler, event callback)
	{
		bool result = false;

		// This will block until the thread is back in the wait condition
		this->cs.lock();
			if (!this->handler)
			{
				this->callback	= callback;
				this->handler	= &handler;
				this->mode		= mode;
				result			= true;
				handler.waiting	= false;
			}
		this->cs.unlock();

		if (result) this->condition.notify_one();

		return result;
	}


	bool Camera::Connect()
	{
		bool result = this->transport.Connected() | this->transport.Connect();

		if (result)
		{
			// Ensure the registers match those in the camera that has just connected.
			result = this->Configure() && this->RefreshRegisters();
			
			if (result)	this->callback(ACQUISITION_CONNECTED);
			else		this->transport.Disconnect();
		}

		return result;
	}


	bool Camera::Connected()
	{
		return this->transport.Connected();
	}
	
	
	bool Camera::Configure()
	{
		this->aliases.clear();
		this->features.clear();
		this->devices.clear();
		this->registers.clear();
		
		vector<byte> devices	= { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		string xml				= chip::v024;
		auto description		= this->devicePool[0xff].Read();
		
		if (description.Size())
		{
			int header = description[0];

			if (header > 1)
			{
				switch (description[1])
				{
					default: break;
				}

				this->monochrome = (description[2] & 0x01) > 0;
			}

			byte *desc	= description + header + 1;
			int count	= *desc++;
			
			devices.assign(desc, desc + count);
		}
		
		for (auto d : devices) 
		{
			this->devices[this->devicePool[d].Name()] = this->devicePool[d];
		}
		
		xml_document doc;

		if (doc.load_buffer(xml.data(), xml.size()))
		{
			return this->Configure(doc.child("camera"));
		}
		
		return false;
	}
	
	
	bool Camera::Configure(xml_node xml)
	{
		this->contextCount = xml.attribute("contexts").as_int(1);
		
		for (auto &child : xml.children("register"))
		{
			int address = strtol(child.attribute("address").as_string("0x00"), nullptr, 0);
			
			this->registers[address] = { child, &this->transport };
		}

		// Two separate loops, because the population of the register list involves
		// growing memory and therefore any references taken for the features would
		// become invalid.
		for (auto &child : xml.children("register"))
		{
			int address = strtol(child.attribute("address").as_string("0x00"), nullptr, 0);
			
			for (auto &feature : child.children("feature"))
			{
				this->features[feature.attribute("name").as_string()] = { feature, &this->registers[address] };
			}
		}

		for (auto &child : xml.children("alias"))
		{
			string key		= child.attribute("name").as_string();
			string feature	= child.attribute("feature").as_string();
			int context		= child.attribute("context").as_int(-1);

			if (!key.empty() && !feature.empty() && this->features.count(feature))
			{
				if (context < 0)
				{
					// If context hasn't been assigned then it should be copied across all contexts
					for (int i=0; i<this->contextCount; i++) 
					{
						this->aliases[i][key] = &this->features[feature];
					}
				}
				else if (context < this->contextCount)
				{
					this->aliases[context][key] = &this->features[feature];
				}
			}
		}
			
		return this->features.size();
	}
		
	
	bool Camera::RefreshRegisters()
	{
		byte maxPage = 0;
		bool waiting = false;
		Buffer<byte> data(512);
		Buffer<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,							// Header
			Commands::ReadRegisterPage, 0x00, 0x00, 0x00, 0x00, 	// Command
			0xff 													// Terminator
		};

		for (auto &r : this->registers) maxPage = max(maxPage, r.second.Page());

		for (byte page = 0; page <= maxPage; page++)
		{
			command[6] = page;

			for (int i=0; i<REFRESH_ATTEMPTS; i++)
			{
				if (this->transport.Transfer(&command, &data, waiting))
				{
					for (auto &r : this->registers)
					{
						if (r.second.Page() == page) r.second.Refresh(data);
					}

					FLOG(info, "Successfully refreshed registers for page %d", page);
					break;
				}
				else FLOG(error, "Failed to refresh registers for page %d", page);
			}
		}
		
		this->context = this->aliases[0]["Context"]->Get();

		return true;
	}


	bool Camera::Grabbing()
	{
		return this->handler;
	}


	bool Camera::Reset()
	{
		return this->transport.Reset();
	}


	void Camera::SetFlash(int power)
	{
		this->flash = power;
	}


	bool Camera::SetContext(byte context)
	{
		if (context < this->contextCount && this->aliases[0].count("Context"))
		{
			this->context = context;
			this->aliases[0]["Context"]->Set(context);

			return true;
		}

		return false;
	}


	int Camera::Capture(DataHandler *image, Mode mode, int flash)
	{
		int width	= this->aliases[this->context]["Width"]->Get();
		int height	= this->aliases[this->context]["Height"]->Get();
		int size	= width * height;
		
		this->receive.Resize(size);
		
		switch (mode)
		{
			case Mode::Normal:			this->send[5] = Commands::Capture;				break;
			case Mode::Master:			this->send[5] = Commands::MasterCapture;		break;
			case Mode::SlaveRising:		this->send[5] = Commands::SlaveCaptureRising;	break;
			case Mode::SlaveFalling:	this->send[5] = Commands::SlaveCaptureFalling;	break;
		}
		
		this->send[6] = (byte)flash;
		this->send[7] = (byte)(size & 0xff);
		this->send[8] = (byte)((size >> 8) & 0xff);
		this->send[9] = (byte)((size >> 16) & 0xff);
		
		if (this->transport.Transfer(&this->send, &this->receive, image->waiting))
		{
			return image->Process(this->monochrome, this->receive, width, height) 
				? ACQUISITION_SUCCESSFUL 
				: ACQUISITION_ERROR_IMAGE_CONVERSION_FAILED;
		}
		
		return ACQUISITION_ERROR_TRANSFER_FAILED;
	}
}


