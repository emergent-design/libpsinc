#include "psinc/Camera.h"
#include "psinc/xml/Devices.h"
#include "psinc/driver/Commands.h"
#include <emergent/logger/Logger.hpp>
#include <emergent/Timer.hpp>

#define REFRESH_ATTEMPTS 3

using namespace std;
using namespace pugi;
using namespace emergent;


namespace psinc
{
	Camera::Camera() : Instrument()
	{
		this->send = Buffer<byte>({
			0x00, 0x00, 0x00, 0x00, 0x00, 	// Header
			0x00, 0x00, 0x00, 0x00, 0x00,	// Command here
			0xff							// Terminator
		});
	}


	void Camera::Initialise(string serial, std::function<void(bool)> onConnection, int timeout)
	{
		this->onConnection = onConnection;

		Instrument::Initialise(Type::Camera, serial, [&](bool connected) {
			this->configured = false;

			if (!connected && this->onConnection)
			{
				this->onConnection(false);
			}
		}, timeout);
	}


	bool Camera::Main()
	{
		if (this->transport.Connected() && !this->configured)
		{
			this->configured = this->Configure() && this->RefreshRegisters();

			if (this->onConnection && this->configured)
			{
				this->onConnection(true);
			}
		}

		if (this->handler)
		{
			bool stream = false;

			if (this->callback)
			{
				if (this->Connected())
				{
					stream = this->callback(this->Capture(this->handler, this->mode, this->flash));
				}
				else
				{
					stream = this->callback(false);

					// Sleep the thread to avoid ramping up processor usage if in streaming mode.
					this_thread::sleep_for(chrono::milliseconds(100));
				}
			}

			if (!stream) this->handler = nullptr;
		}

		return this->handler == nullptr;
	}


	bool Camera::GrabImage(Mode mode, DataHandler &handler, std::function<bool(bool)> callback)
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


	bool Camera::Connected()
	{
		return this->transport.Connected() && this->configured;
	}


	bool Camera::Configure()
	{
		this->aliases.clear();
		this->features.clear();
		this->registers.clear();

		xml_document doc;
		auto xml			= chip::v024;
		auto description	= this->CustomDevice(0xff).Read(); //this->devicePool[0xff].Read();

		if (description.Size())
		{
			int header = description[0];

			if (header > 1)
			{
				switch (description[1])
				{
					case 0x00:	xml = chip::v024;	break;
					case 0x01:	xml = chip::mt9;	break;
					default: 						break;
				}

				this->monochrome = (description[2] & 0x01) == 0;
			}
		}

		return doc.load(xml) && this->Configure(doc.child("camera"));
	}


	bool Camera::Configure(xml_node xml)
	{
		this->contextCount	= xml.attribute("contexts").as_int(1);
		this->hdr			= xml.attribute("hdr").as_bool();
		this->sizeByRange	= xml.attribute("sizeByRange").as_bool();
		this->addressSize	= xml.attribute("addressSize").as_int(1);
		this->bayerMode		= xml.attribute("bayer").as_int(0);

		for (auto &child : xml.children("register"))
		{
			int address = strtol(child.attribute("address").as_string("0x00"), nullptr, 0);

			this->registers[address] = { child, &this->transport, this->addressSize };
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
		atomic<bool> waiting(false);

		byte minPage = 255;
		byte maxPage = 0;
		Buffer<byte> data(512);
		Buffer<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,							// Header
			Commands::ReadRegisterPage, 0x00, 0x00, 0x00, 0x00, 	// Command
			0xff 													// Terminator
		};

		for (auto &r : this->registers)
		{
			minPage = min(minPage, r.second.Page());
			maxPage = max(maxPage, r.second.Page());
		}

		for (byte page = minPage; page <= maxPage; page++)
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

					Log::Info("Successfully refreshed registers for page %d", page);
					break;
				}
				else Log::Error("Failed to refresh registers for page %d", page);
			}
		}

		this->context = this->aliases[0]["Context"]->Get();

		return true;
	}


	bool Camera::SetRegister(int address, int value)
	{
		atomic<bool> waiting(false);

		Buffer<byte> data = {
			0x00, 0x00, 0x00, 0x00, 0x00, 			// Header
			Commands::WriteRegister, 				// Command
			(byte)(address & 0xff),
			(byte)((address >> 8) & 0xff),
			(byte)(value & 0xff),
			(byte)((value >> 8) & 0xff),
			0xff									// Terminator
		};

		return this->transport.Transfer(&data, nullptr, waiting);
	}


	int Camera::GetRegister(int address)
	{
		atomic<bool> waiting(false);
		Buffer<byte> receive(5);
		Buffer<byte> command = {
			0x00, 0x00, 0x00, 0x00, 0x00,																	// Header
			0x03, 0x00, 0x00, 0x00, 0x00, 																	// Flush command
			Commands::QueueRegister, (byte)(address & 0xff), (byte)((address >> 8) & 0xff), 0x00, 0x00, 	// Command
			0xff 																							// Terminator
		};

		if (this->transport.Transfer(&command, &receive, waiting))
		{
			return (receive[4] << 8) + receive[3];
		}

		return -1;
	}


	bool Camera::Grabbing()
	{
		return this->handler;
	}


	void Camera::SetFlash(byte power)
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


	bool Camera::SetWindow(byte context, int x, int y, int width, int height)
	{
		if (!this->aliases[context].count("ColumnStart"))	return false;
		if (!this->aliases[context].count("RowStart"))		return false;

		int mx = this->aliases[context]["ColumnStart"]->Minimum();
		int my = this->aliases[context]["RowStart"]->Minimum();

		this->aliases[context]["ColumnStart"]->Set(mx + x);
		this->aliases[context]["RowStart"]->Set(my + y);

		if (this->sizeByRange)
		{
			if (width < 0)	this->aliases[context]["ColumnEnd"]->Reset();
			else			this->aliases[context]["ColumnEnd"]->Set(mx + x + width - 1);

			if (height < 0)	this->aliases[context]["RowEnd"]->Reset();
			else			this->aliases[context]["RowEnd"]->Set(my + y + height - 1);
		}
		else
		{
			if (width < 0)	this->aliases[context]["Width"]->Reset();
			else			this->aliases[context]["Width"]->Set(width);

			if (height < 0)	this->aliases[context]["Height"]->Reset();
			else			this->aliases[context]["Height"]->Set(height);
		}

		return true;
	}


	bool Camera::Capture(DataHandler *handler, Mode mode, int flash)
	{
		int width, height;

		if (this->sizeByRange)
		{
			width	= this->aliases[this->context]["ColumnEnd"]->Get() - this->aliases[this->context]["ColumnStart"]->Get() + 1;
			height	= this->aliases[this->context]["RowEnd"]->Get() - this->aliases[this->context]["RowStart"]->Get() + 1;
		}
		else
		{
			width	= this->aliases[this->context]["Width"]->Get();
			height	= this->aliases[this->context]["Height"]->Get();
		}

		int size = width * height * (this->hdr ? 2 : 1);

		this->receive.Resize(size);

		switch (mode)
		{
			case Mode::Normal:			this->send[5] = Commands::Capture;				break;
			case Mode::Master:			this->send[5] = Commands::MasterCapture;		break;
			case Mode::SlaveRising:		this->send[5] = Commands::SlaveCaptureRising;	break;
			case Mode::SlaveFalling:	this->send[5] = Commands::SlaveCaptureFalling;	break;
		}

		this->send[6] = flash;
		this->send[7] = (byte)(size & 0xff);
		this->send[8] = (byte)((size >> 8) & 0xff);
		this->send[9] = (byte)((size >> 16) & 0xff);

		return this->transport.Transfer(&this->send, &this->receive, handler->waiting)
			&& handler->Process(this->monochrome, this->hdr, this->receive, width, height, this->bayerMode);
	}
}
