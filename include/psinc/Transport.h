#pragma once

#include <emergent/Emergent.hpp>
#include <emergent/struct/Buffer.hpp>
// #include <psinc/TransportBuffer.hpp>
#include <libusb-1.0/libusb.h>
#include <atomic>
#include <mutex>
#include <queue>
#include <map>
#include <set>


namespace psinc
{
	using emg::byte;

	/// USB transport implementation.
	///
	/// Uses libusb to connect to and communicate with the physical
	/// devices. This supports connecting to specific hardware based
	/// on the bus or a serial number regex.
	class Transport
	{
		public:

			Transport();
			virtual ~Transport();

			/// Initialises the transport with the product ID and
			/// serial of interest.
			/// The serial string will be treated as a regex expression.
			/// The netchip flag will enable the legacy vendor ID for older cameras
			bool Initialise(const std::set<uint16_t> &vendors, uint16_t product, std::string serial, std::function<void(bool)> onConnection, int timeout = 500);


			void SetTimeout(int timeout);


			/// Poll to allow hotplug detection to work.
			void Poll(int time);


			/// Report whether or not the transport is connected.
			bool Connected() const;


			/// Transfer packets to and from the actual device
			bool Transfer(emg::Buffer<byte> *send, emg::Buffer<byte> *receive, std::atomic<bool> &waiting, bool check = true, bool truncate = false);


			/// Reset the connection to the actual device.
			bool Reset(bool control = false);


			/// Force a disconnection of this device (if connected)
			/// No disconnect event will be emitted in this case.
			void Disconnect();


			/// Returns the USB major version of the connection and 0 if no device is connected
			uint8_t UsbVersion() const;


			/// Return a list of serial numbers and product descriptions for all
			/// connected devices that match the given product ID.
			static std::map<std::string, std::string> List(const std::set<uint16_t> &vendors, uint16_t product);


		private:

			/// Method for handling connection/disconnection events.
			void Pending(libusb_device *device, libusb_hotplug_event event);

			/// Retrieve the serial number of the device (then stored as id).
			/// If a particular pattern is required then return true if there
			/// is a match.
			bool Match(libusb_device_handle *device, int index);


			/// In the case where hotplug is not supported (looking at you Windows),
			/// attempt a more expensive manual enumeration and connection. This can
			/// be removed once Windows hotplugging support has been implemented.
			void LegacyConnect();


			/// Attempt to claim the given device.
			bool Claim(libusb_device *device);


			/// Check that the device matches an expected vendor and product ID.
			static bool Valid(libusb_device *device, const std::set<uint16_t> &vendors, uint16_t product);


			/// Tranfer the given data to the device (write) or from the device (!write)
			bool Transfer(emg::Buffer<byte> *buffer, bool write, bool check, bool truncate);


			/// Releases the device.
			void Release();


			/// List of supported vendor IDs
			std::set<uint16_t> vendors;

			/// Product ID that we are interested in
			uint16_t product;

			/// Full serial of the device connected to by this transport instance.
			std::string id;

			/// Mutex used to prevent asynchronous calls to Transfer or Reset from
			/// breaking things
			std::mutex cs;

			/// The serial number of interest, supports a regex string. If empty
			/// the transport will connect to the first device it can find.
			std::string serial;

			/// The libusb handle to the actual device (if this transport has successfully
			/// claimed one)
			libusb_device_handle *handle = nullptr;

			/// A thread specific context for dealing with libusb
			libusb_context *context = nullptr;

			/// Required timeout for bulk transfers
			int timeout = 500;

			/// Reference for registering hotplugging callback
			libusb_hotplug_callback_handle hotplug;

			/// Indicates whether or not a hotplug callback has been previously registered.
			bool registered = false;

			/// Storage for the registered callback (if required)
			std::function<void(bool)> onConnection = nullptr;

			std::queue<libusb_device *> pending;

			// Disconnection can happen for a number of reasons so use a flag to indicate
			// that the onConnection event requires triggering at the next opportunity.
			bool disconnect = false;

			// Windows does not yet support hotplugging so adapt accordingly
			bool legacy = false;

			// USB major version
			uint8_t version = 0;

			/// Attempt to allocate a DMA read buffer for bulk transfers
			// TransportBuffer readBuffer;

			/// Allows an internal global function to push onto the pending queue
			friend int LIBUSB_CALL OnHotplug(libusb_context *context, libusb_device *device, libusb_hotplug_event event, void *data);
	};
}
