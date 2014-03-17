#pragma once

#include <emergent/Emergent.h>
#include <emergent/struct/Buffer.h>
#include <libusb-1.0/libusb.h>
//#include <map>
#include <mutex>

/// USB transport implementation.
///
/// Uses libusb to connect to and communicate with the physical
/// devices. This handles thread-safe claiming of devices and maintains
/// a list of claimed devices (based on the device bus address)
class Transport
{
	public:

		Transport();
		virtual ~Transport();

		/// Initialises the transport with the given configuration information (supplied
		/// by the device instance). Will return false if unable to initialise the transport.
		/// The serial string will be treated as a regex expression.
		bool Initialise(std::string serial, int bus);
		
		/// Attempt to connect to the device if not already connected.
		bool Connect();
		
		/// Report whether or not the transport is connected.
		bool Connected();

		/// Transfer packets to and from the actual device
		bool Transfer(emg::Buffer<byte> *send, emg::Buffer<byte> *receive, volatile bool &waiting, bool check = true);

		/// Reset the connection to the actual device
		bool Reset();
		
		void Disconnect();
	
	
	private:
		std::string Serial(libusb_device_handle *device, int index);
		
		/// Attempt to claim the given device. This will check that the
		/// device has not already been claimed and that it is actually
		/// able to take control of the it.
		bool Claim(libusb_device *device, int index, std::string serial);

		

		/// Tranfer the given data to the device (write) or from
		/// the device (!write)
		bool Transfer(emg::Buffer<byte> *buffer, bool write, bool check);
		
		
		/// Releases the device.
		void Release();


		/// Mutex used to prevent asynchronous calls to Transfer or Reset from
		/// breaking things
		std::mutex cs;
		
		int bus;
		std::string serial;

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
