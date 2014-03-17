#include "psinc/transport/Transport.h"
#include <emergent/Logger.h>
#include <regex>

using namespace std;
using namespace emergent;

#define USB_TIMEOUT 500
#define VENDOR		0x0525
#define PRODUCT		0xaaca
#define WRITE_PIPE	0x03
#define READ_PIPE	0x81


//set<unsigned int> Transport::claimed;
//mutex Transport::csClaim;


Transport::Transport()
{
	libusb_init(&this->context);
}


Transport::~Transport()
{
	this->Release();
	
	libusb_exit(this->context);
}


bool Transport::Initialise(string serial, int bus)
{
	this->serial	= serial;
	this->bus		= bus;

	return true;
}


bool Transport::Connect()
{
	lock_guard<mutex> lock(this->cs);
	
	if (!this->handle)
	{
		libusb_device **list;
		libusb_device_descriptor descriptor;

		libusb_get_device_list(this->context, &list);

		// Loop through the list of connected USB devices
		for (libusb_device **device = list; *device; device++)
		{
			if (libusb_get_device_descriptor(*device, &descriptor) == 0)
			{
				if (descriptor.idVendor == VENDOR && descriptor.idProduct == PRODUCT)
				{
					// If a particular device matches the known vendor, product ID then attempt to claim it.
					if (this->Claim(*device, descriptor.iSerialNumber, this->serial)) break;
				}
			}
		}
		
		libusb_free_device_list(list, 1);
	}
	
	return this->handle;
}


bool Transport::Connected()
{
	return this->handle;
}


string Transport::Serial(libusb_device_handle *device, int index)
{
	unsigned char data[64];
	
	libusb_get_string_descriptor_ascii(device, index, data, 64);
	
	return string(reinterpret_cast<char *>(data));
}


bool Transport::Claim(libusb_device *device, int index, string serial)
{
	//lock_guard<mutex> lock(csClaim);

	unsigned int bus = libusb_get_bus_number(device);
	
	// If a bus has been specified (non-zero) then check this device lives on that bus
	if (!this->bus || this->bus == bus)
	{
		// Generate an ID by combining the bus number and address of the device
		this->id = (bus << 8) | libusb_get_device_address(device);

		// Check if the ID already exists in the list, if it does then this device
		// has already been claimed, time to leave.
		//if (claimed.count(this->id) == 0)
		{
			// Attempt to open and claim the device
			if (libusb_open(device, &this->handle) == 0)
			{
				// If a serial number has been specified then check that this device matches it
				if (serial.empty() || regex_match(this->Serial(this->handle, index), regex(serial)))
				{
					// TODO: Disabled this while Ashton is solving an issue with
					// the camera that causes it to get stuck in a strange state.
					//if (libusb_set_configuration(this->handle, 1) == 0)
					{
						if (libusb_claim_interface(this->handle, 0) == 0)
						{
							// If the device was claimed successfully then added it
							// to the list and return triumphant.
							FLOG(info, "USB device claimed: %d", this->id);
							//claimed.insert(this->id);
							return true;
						}
					}
				}

				// Claiming was unsuccessful so clean up
				libusb_close(this->handle);
				this->handle = nullptr;
			}
		}
	}

	return false;
}


void Transport::Disconnect()
{
	lock_guard<mutex> lock(this->cs);
	
	this->Release();
}


void Transport::Release()
{
	if (this->handle)
	{
		// Release the device and close the handle
		libusb_release_interface(this->handle, 0);
		libusb_close(this->handle);

		this->handle = nullptr;

		FLOG(info, "USB deviced released: %d", this->id);
	}
}


bool Transport::Reset()
{
	lock_guard<mutex> lock(this->cs);

	if (this->handle)
	{
		if (libusb_reset_device(this->handle) != 0)
		{
			// Something has gone wrong with the reset and so the device appears
			// as if it has been reconnected. Therefore this handle is invalid
			// and must be cleaned up.
			this->Release();

			LOG(error, "Problem resetting USB device, handle has been released");
		}
	}

	return this->handle;
}


bool Transport::Transfer(Buffer<byte> *send, Buffer<byte> *receive, volatile bool &waiting, bool check)
{
	lock_guard<mutex> lock(this->cs);

	return this->handle ? this->Transfer(send, true, check) && (waiting = true) && this->Transfer(receive, false, check) : false;
}


bool Transport::Transfer(Buffer<byte> *buffer, bool write, bool check)
{
	if (buffer)
	{
		int transferred;
		bool result	= false;
		int err		= libusb_bulk_transfer(this->handle, write ? WRITE_PIPE : READ_PIPE, *buffer, buffer->Size(), &transferred, USB_TIMEOUT);

		if (!err)
		{
			//cout << (write ? "w: " : "r: ") << transferred << " - " << *buffer << endl;
			result = (write || check) ? transferred == buffer->Size() : true;

			if (!result) FLOG(error, "USB device %d: Incomplete transfer when %s", this->id, write ? "writing" : "reading");
		}
		else if (err == LIBUSB_ERROR_NO_DEVICE)
		{
			FLOG(error, "USB device %d: Device has been disconnected", this->id);
			this->Release();
		}
		else FLOG(error, "USB device %d: Transfer error %s when %s", this->id, err, write ? "writing" : "reading");

		return result;
	}

	return true;
}
