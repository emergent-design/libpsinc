#pragma once

#pragma once

#include <emergent/Emergent.h>
#include <psinc/transport/Transport.h>
#include <emergent/struct/Buffer.h>
#include <libusb-1.0/libusb.h>
#include <mutex>


namespace psinc
{
	/// USB transport implementation.
	///
	/// Uses libusb to connect to and communicate with the physical
	/// devices. This supports connecting to specific hardware based
	/// on the bus or a serial number regex.
	/// This is a test of the new libusb hotplug features and should
	/// be migrated into the main Transport if successful.
	class TransportHotplug : public Transport
	{
		public:

			virtual ~TransportHotplug();

			/// Initialises the transport with the product ID and
			/// serial of interest.
			/// The serial string will be treated as a regex expression.
			bool Initialise(int product, std::string serial, std::function<void(bool)> onConnection);


			void Poll(int time);


		private:


			bool Match(libusb_device_handle *device, int index);


			/// Attempt to claim the given device.
			bool Claim(libusb_device *device);

			/// Releases the device.
			virtual void Release();


			void OnHotplug(libusb_device *device, libusb_hotplug_event event);


			/// Product ID that we are interested in
			int product;

			/// ID of the device connected to by this transport instance. Formed by
			/// combining the bus number and address.
			std::string id;


			libusb_hotplug_callback_handle hotplug;
			bool registered = false;

			std::function<void(bool)> onConnection;
	};
}
