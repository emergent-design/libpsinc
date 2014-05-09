#include "psinc/transport/TransportHotplug.h"
#include <emergent/Logger.h>
#include <regex>

using namespace std;
using namespace emergent;

#define VENDOR		0x0525	// PSI
#define WRITE_PIPE	0x03
#define READ_PIPE	0x81


namespace psinc
{
	TransportHotplug::TransportHotplug()
	{
		libusb_init(&this->context);
	}


	TransportHotplug::~TransportHotplug()
	{
		this->Release();

		libusb_hotplug_deregister_callback(this->context, this->hotplug);
		libusb_exit(this->context);
	}


	bool TransportHotplug::Initialise(int product, string serial, std::function<void(bool)> onConnection)
	{
		if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG))
		{
			LOG(error, "USB Hotplug capabilites are not supported on this platform");
			return false;
		}

		this->serial		= serial;
		this->product		= product;
		this->onConnection	= onConnection;


		libusb_hotplug_deregister_callback(this->context, this->hotplug);

		this->registered = false;

		return true;
	}


	void TransportHotplug::Poll(int time)
	{
		if (!this->registered)
		{
			auto events			= (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT);
			auto callback		= [](libusb_context *context, libusb_device *device, libusb_hotplug_event event, void *data) {
				((TransportHotplug *)data)->OnHotplug(device, event);
				return 0;
			};

			if (libusb_hotplug_register_callback(this->context, events, LIBUSB_HOTPLUG_ENUMERATE, VENDOR, this->product, LIBUSB_HOTPLUG_MATCH_ANY, callback, this, &this->hotplug))
			{
				LOG(error, "Unable to register USB hotplug callback");
				return;
			}

			this->registered = true;
		}

		struct timeval tv = { 0, time * 1000 };
		libusb_handle_events_timeout_completed(this->context, &tv, nullptr);
	}


	void TransportHotplug::OnHotplug(libusb_device *device, libusb_hotplug_event event)
	{
		bool trigger = false;
		//lock_guard<mutex> lock(this->cs);
		this->cs.lock();

		if (this->handle)
		{
			if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)
			{
				if (device == libusb_get_device(this->handle))
				{
					this->Release();
					trigger = true;
				}
			}
		}
		else
		{
			if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
			{
				if (this->Claim(device) && this->onConnection)
				{
					trigger = true;
					//this->onConnection(true);
				}
			}
		}

		this->cs.unlock();

		if (trigger && this->onConnection) this->onConnection(this->handle);
	}


	bool TransportHotplug::Match(libusb_device_handle *device, int index)
	{
		if (!this->serial.empty())
		{
			unsigned char data[64];

			libusb_get_string_descriptor_ascii(device, index, data, 64);

			return regex_match(reinterpret_cast<char *>(data), regex(this->serial));
		}

		return true;
	}


	bool TransportHotplug::Claim(libusb_device *device)
	{
		libusb_device_descriptor descriptor;

		if (libusb_get_device_descriptor(device, &descriptor) == 0)
		{
			// Attempt to open and claim the device
			if (libusb_open(device, &this->handle) == 0)
			{
				// If a serial number has been specified then check that this device matches it
				if (this->Match(this->handle, descriptor.iSerialNumber))
				{
					// TODO: Disabled this while PSI is solving an issue with
					// the camera that causes it to get stuck in a strange state.
					//if (libusb_set_configuration(this->handle, 1) == 0)
					{
						if (libusb_claim_interface(this->handle, 0) == 0)
						{
							this->id = tfm::format("%03d-%03d", libusb_get_bus_number(device), libusb_get_device_address(device));

							FLOG(info, "USB device claimed: %s", this->id);

							return true;
						}
					}
				}

				// Claiming was unsuccessful so clean up
				libusb_close(this->handle);
				this->handle 	= nullptr;
				this->id		= "";
			}
		}

		return false;
	}



	void TransportHotplug::Release()
	{
		if (this->handle)
		{
			// Release the device and close the handle
			libusb_release_interface(this->handle, 0);
			libusb_close(this->handle);

			FLOG(info, "USB deviced released: %s", this->id);

			this->handle	= nullptr;
			this->id		= "";
		}
	}

}

