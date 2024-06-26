#include "psinc/Transport.h"
#include <emergent/logger/Logger.hpp>
#include <emergent/String.hpp>
#include <regex>
// #include <cstring>

#define WRITE_PIPE	0x03
#define READ_PIPE	0x81

using std::string;


// #include <emergent/Timer.hpp>

namespace psinc
{
	Transport::Transport()
	{
		libusb_init(&this->context);

		this->legacy = !libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG);
	}


	Transport::~Transport()
	{
		this->Release();

		if (!this->legacy)
		{
			libusb_hotplug_deregister_callback(this->context, this->hotplug);
		}

		libusb_exit(this->context);
	}


	bool Transport::Initialise(const std::set<uint16_t> &vendors, uint16_t product, std::string serial, std::function<void(bool)> onConnection, int timeout)
	{
		this->Disconnect();

		this->vendors		= vendors;
		this->product		= product;
		this->serial		= serial;
		this->onConnection	= onConnection;
		this->timeout		= timeout;

		if (!this->legacy)
		{
			libusb_hotplug_deregister_callback(this->context, this->hotplug);
			this->registered = false;
		}
		else emg::Log::Info("USB hotplug not supported on this platform, running in legacy mode");

		return true;
	}


	void Transport::SetTimeout(int timeout)
	{
		this->timeout = timeout;
	}


	bool Transport::Connected() const
	{
		return this->handle;
	}


	uint8_t Transport::UsbVersion() const
	{
		return version;
	}


	int LIBUSB_CALL OnHotplug(libusb_context *, libusb_device *device, libusb_hotplug_event event, void *data)
	{
		reinterpret_cast<Transport *>(data)->Pending(device, event);
		return 0;
	}


	void Transport::Pending(libusb_device *device, libusb_hotplug_event event)
	{
		if (Valid(device, this->vendors, this->product))
		{
			if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT && this->handle)
			{
				if (device == libusb_get_device(this->handle))
				{
					this->cs.lock();
						this->Release();
					this->cs.unlock();
				}
			}

			if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED && !this->handle)
			{
				this->pending.push(device);
			}
		}
	}


	void Transport::Poll(int time)
	{
		if (this->disconnect && this->onConnection)
		{
			this->onConnection(false);
			this->disconnect = false;
		}

		if (this->legacy)
		{
			if (!this->handle)
			{
				this->LegacyConnect();
			}

			return;
		}

		if (!this->registered)
		{
			auto events = (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT);

			if (libusb_hotplug_register_callback(this->context, events, LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY, this->product, LIBUSB_HOTPLUG_MATCH_ANY, &OnHotplug, this, &this->hotplug))
			{
				emg::Log::Error("Unable to register USB hotplug callback");
				return;
			}

			this->registered = true;
		}

		struct timeval tv = { 0, time * 1000 };
		libusb_handle_events_timeout_completed(this->context, &tv, nullptr);

		while (!this->pending.empty())
		{
			if (!this->handle && this->Claim(this->pending.front()) && this->onConnection)
			{
				this->onConnection(true);
				this->disconnect = false;
			}
			this->pending.pop();
		}
	}


	bool Transport::Match(libusb_device_handle *device, int index)
	{
		unsigned char data[128];

		libusb_get_string_descriptor_ascii(device, index, data, 128);

		this->id = emg::String::trim(reinterpret_cast<char *>(data), ' ');

		return this->serial.empty() ? true : regex_match(this->id, std::regex(this->serial));
	}


	bool Transport::Valid(libusb_device *device, const std::set<uint16_t> &vendors, uint16_t product)
	{
		libusb_device_descriptor descriptor;

		return libusb_get_device_descriptor(device, &descriptor) == 0
			&& vendors.count(descriptor.idVendor)
			&& descriptor.idProduct == product;
	}


	void Transport::LegacyConnect()
	{
		libusb_device **list;
		libusb_get_device_list(this->context, &list);

		// Loop through the list of connected USB devices
		for (libusb_device **device = list; *device; device++)
		{
			if (Valid(*device, this->vendors, this->product))
			{
				// If a particular device matches the known vendor, product ID then attempt to claim it.
				if (this->Claim(*device))
				{
					if (this->onConnection) this->onConnection(true);
					break;
				}
			}
		}

		libusb_free_device_list(list, 1);
	}


	bool Transport::Claim(libusb_device *device)
	{
		std::lock_guard lock(this->cs);

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
							this->version = (descriptor.bcdUSB >> 8) & 0xff;

							emg::Log::Info(
								"%u: USB (v%d.%d) device claimed - %s",
								emg::Timestamp::LogTime(),
								this->version,
								descriptor.bcdUSB & 0xff,
								this->id);
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


	std::map<string, Transport::Info> Transport::List(const std::set<uint16_t> &vendors, uint16_t product)
	{
		std::map<string, Info> result;

		unsigned char data[128];
		libusb_device **list;
		libusb_device_descriptor descriptor;
		libusb_device_handle *handle;
		libusb_context *context = nullptr;

		libusb_init(&context);
		libusb_get_device_list(context, &list);

		for (libusb_device **device = list; *device; device++)
		{
			if (Valid(*device, vendors, product) && libusb_get_device_descriptor(*device, &descriptor) == 0)
			{
				if (libusb_open(*device, &handle) == 0)
				{
					libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber, data, 128);
					string serial = emg::String::trim(reinterpret_cast<char *>(data), ' ');

					libusb_get_string_descriptor_ascii(handle, descriptor.iProduct, data, 128);
					result[serial] = {
						reinterpret_cast<char *>(data),
						emg::String::format(
							"v%d.%d",
							(descriptor.bcdUSB >> 8) & 0xff,
							descriptor.bcdUSB & 0xff
						),
						emg::String::format(
							"%d:%d:%d",
							libusb_get_bus_number(*device),
							libusb_get_port_number(*device),
							libusb_get_device_address(*device)
						)
					};

					libusb_close(handle);
				}
			}
		}

		libusb_free_device_list(list, 1);
		libusb_exit(context);

		return result;

	}


	void Transport::Disconnect()
	{
		std::lock_guard lock(this->cs);

		this->Release();
	}


	void Transport::Release()
	{
		if (this->handle)
		{
			// this->readBuffer.Dispose();

			// Release the device and close the handle
			libusb_release_interface(this->handle, 0);
			libusb_close(this->handle);

			emg::Log::Info("%u: USB deviced released - %s", emg::Timestamp::LogTime(), this->id);

			this->disconnect	= true;
			this->handle		= nullptr;
			this->id			= "";
			this->version		= 0;
		}
	}



	bool Transport::Reset(bool control)
	{
		std::lock_guard lock(this->cs);

		if (this->handle)
		{
			if (control)
			{
				libusb_control_transfer(this->handle, 0x40, 0xf2, 0, 0, 0, 0, this->timeout);

				// This tells the camera to reset itself and will therefore result in
				// disconnection so assume that the handle is now invalid.
				this->Release();
			}
			else if (libusb_reset_device(this->handle) != 0)
			{
				// Something has gone wrong with the reset and so the device appears
				// as if it has been reconnected. Therefore this handle is invalid
				// and must be cleaned up.
				this->Release();

				emg::Log::Error("Problem resetting USB device, handle has been released");
			}
		}

		return this->handle;
	}


	bool Transport::Transfer(std::vector<byte> *send, std::vector<byte> *receive, std::atomic<bool> &waiting, bool check, bool truncate)
	{
		std::lock_guard lock(this->cs);

		return this->handle
			? this->Transfer(send, true, check, false)
				&& (waiting = true)
				&& this->Transfer(receive, false, check, truncate)
			: false;
	}


	bool Transport::Transfer(std::vector<byte> *buffer, bool write, bool check, bool truncate)
	{
		if (buffer)
		{
			// if (!write)
			// {
			// 	this->readBuffer.Resize(this->handle, buffer->size());
			// }

			// emg::Timer timer;

			int transferred	= 0;
			bool result		= false;
			int err			= write
				? libusb_bulk_transfer(this->handle, WRITE_PIPE, buffer->data(), buffer->size(), &transferred, this->timeout)
				: libusb_bulk_transfer(this->handle, READ_PIPE, buffer->data(), buffer->size(), &transferred, this->timeout);
				// : libusb_bulk_transfer(this->handle, READ_PIPE, this->readBuffer.Data(), this->readBuffer.Size(), &transferred, this->timeout);

			// const auto time = timer.MicroElapsed();

			if (!err)
			{
				result = (write || check) ? transferred == (int)buffer->size() : true;


				// if (!write && result)
				// {
				// 	// When requested, truncate the buffer to the size of data actually received.
				// 	// buffer->assign(this->readBuffer.Data(), truncate ? transferred : this->readBuffer.Size());
				// 	// buffer->assign(this->readBuffer.Data(), this->readBuffer.Data() + (truncate ? transferred : this->readBuffer.Size()));
				// 	std::memcpy(buffer->data(), this->readBuffer.Data(), truncate ? transferred : this->readBuffer.Size());
				// }

				if (!write && result && truncate)
				{
					buffer->resize(transferred);
				}

				if (!result)
				{
					emg::Log::Error(
						"%u: USB device %s - Incomplete transfer when %s (%d of %d bytes)",
						emg::Timestamp::LogTime(),
						this->id,
						write ? "writing" : "reading",
						transferred,
						buffer->size()
					);
				}
			}
			else if (this->legacy && (err == LIBUSB_ERROR_NO_DEVICE || err == LIBUSB_ERROR_IO))
			{
				emg::Log::Error("%u: USB device %s - Device has been disconnected", emg::Timestamp::LogTime(), this->id);
				this->Release();
			}
			else
			{
				// std::cout << "time spent transferring: " << time << "us\n";
				emg::Log::Error("%u: USB device %s - %s (%d) when %s (%d bytes transferred)", emg::Timestamp::LogTime(), this->id, libusb_error_name(err), err, write ? "writing" : "reading", transferred);
			}

			return result;
		}

		return true;
	}
}

