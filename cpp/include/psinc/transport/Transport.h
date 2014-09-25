#pragma once

#include <emergent/Emergent.h>
#include <emergent/struct/Buffer.h>
#include <libusb-1.0/libusb.h>
#include <mutex>


namespace psinc
{
	/// USB transport implementation.
	///
	/// Uses libusb to connect to and communicate with the physical
	/// devices. This supports connecting to specific cameras based
	/// on the bus or a serial number regex.
	class Transport
	{
		public:

			Transport();
			virtual ~Transport();

			/// Initialises the transport with the serial and/or bus number
			/// of interest. The serial string will be treated as a regex expression.
			bool Initialise(std::string serial, int bus);


			/// Attempt to connect to the device if not already connected.
			bool Connect();


			/// Report whether or not the transport is connected.
			bool Connected();


			/// Transfer packets to and from the actual device
			bool Transfer(emg::Buffer<byte> *send, emg::Buffer<byte> *receive, volatile bool &waiting, bool check = true);


			/// Reset the connection to the actual device
			bool Reset();


			/// Force a disconnection of this device (if connected)
			void Disconnect();


		protected:

			/// Read the serial number of the device descriptor. A PSI camera will
			/// concatenate the assigned name to the serial number so can be used
			/// to identify a camera in a customisable way.
			std::string Serial(libusb_device_handle *device, int index);


			/// Attempt to claim the given device.
			bool Claim(libusb_device *device, int index, std::string serial);


			/// Tranfer the given data to the device (write) or from the device (!write)
			bool Transfer(emg::Buffer<byte> *buffer, bool write, bool check);


			/// Releases the device.
			virtual void Release();


			/// Mutex used to prevent asynchronous calls to Transfer or Reset from
			/// breaking things
			std::mutex cs;

			/// The bus number of interest (or 0 for any bus)
			int bus;

			/// The serial number of interest, supports a regex string. If empty
			/// the transport will connect to the first device it can find.
			std::string serial;

			/// ID of the device connected to by this transport instance. Formed by
			/// combining the bus number and address.
			unsigned int id;

			/// The libusb handle to the actual device (if this transport has successfully
			/// claimed one)
			libusb_device_handle *handle = nullptr;

			/// A thread specific context for dealing with libusb
			libusb_context *context = nullptr;

			int timeout = 500;
	};
}
