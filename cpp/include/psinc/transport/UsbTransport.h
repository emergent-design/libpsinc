#pragma once

/*
#include <libusb-1.0/libusb.h>
#include "psinc/transport/Transport.h"
#include <mutex>
#include <set>

/// USB transport implementation.
///
/// Uses libusb to connect to and communicate with the physical
/// devices. This handles thread-safe claiming of devices and maintains
/// a list of claimed devices (based on the device bus address)
class UsbTransport : public Transport
{
	public:
		/// Default constructor
		UsbTransport();

		/// Destructor
		virtual ~UsbTransport();

		// Refer to parent documentation
		virtual bool Initialise(std::map<std::string, int> &config, std::string serial, int bus);

		// Refer to parent documentation
        virtual bool Transfer(emg::Buffer<byte> *send, emg::Buffer<byte> *receive, volatile bool &waiting, bool check = true);

		// Refer to parent documentation
        virtual bool Reset();
		
		virtual bool Connect();
		
		virtual bool Connected();

	protected:
	private:
		std::string Serial(libusb_device_handle *device, int index);
		
		/// Attempt to claim the given device. This will check that the
		/// device has not already been claimed and that it is actually
		/// able to take control of the it.
		bool Claim(libusb_device *device, int index, std::string serial);

		/// Releases the device and removes it from the list.
		void Release();

		/// Tranfer the given data to the device (write) or from
		/// the device (!write)
		bool Transfer(emg::Buffer<byte> *buffer, bool write, bool check);


		/// Mutex used to prevent asynchronous calls to Transfer or Reset from
		/// breaking things
		std::mutex cs;
		
		int vendor;
		int product;
		int bus;
		std::string serial;

		/// Storage for the read pipe address
		byte readPipe;

		/// Storage for the write pipe address
		byte writePipe;

		/// ID of the device connected to by this transport instance. Formed by
		/// combining the bus number and address.
		unsigned int id;

		/// The libusb handle to the actual device (if this transport has successfully
		/// claimed one)
		libusb_device_handle *handle	= nullptr;
		libusb_context *context			= nullptr;

		/// Static unique list of claimed devices. The values in the set are the IDs
		/// (see above) of claimed devices. This is used when claiming to prevent a
		/// device from being claimed twice which is permitted by libusb but undesirable
		/// in this situation.
		//static std::set<unsigned int> claimed;

		/// Mutex used to protect the list of claimed devices
		//static std::mutex csClaim;
};
*/