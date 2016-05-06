#pragma once

#include <emergent/Emergent.hpp>
#include <emergent/struct/Buffer.hpp>
#include <emergent/concurrentqueue.h>
// #include <psinc/transport/Transport.h>
#include <libusb-1.0/libusb.h>
#include <mutex>
#include <queue>


namespace psinc
{
	/// USB transport implementation.
	///
	/// Uses libusb to connect to and communicate with the physical
	/// devices. This supports connecting to specific hardware based
	/// on the bus or a serial number regex.
	/// This is a test of the new libusb hotplug features and should
	/// be migrated into the main Transport if successful.
	class Transport //: public Transport
	{
		public:

			Transport();
			virtual ~Transport();

			/// Initialises the transport with the product ID and
			/// serial of interest.
			/// The serial string will be treated as a regex expression.
			bool Initialise(int product, std::string serial, std::function<void(bool)> onConnection, int timeout = 500);


			/// Poll to allow hotplug detection to work.
			void Poll(int time);


			/// Report whether or not the transport is connected.
			bool Connected();


			/// Transfer packets to and from the actual device
			bool Transfer(emg::Buffer<byte> *send, emg::Buffer<byte> *receive, std::atomic<bool> &waiting, bool check = true, bool truncate = false);


			/// Reset the connection to the actual device.
			bool Reset();


			/// Force a disconnection of this device (if connected)
			/// No disconnect event will be emitted in this case.
			void Disconnect();


			/// Return a list of serial numbers for all connected devices
			/// that match the given product ID.
			static std::vector<std::string> List(int product);


		private:

			/// Simple structure for queueing up connection/disconnection
			/// events for handling in Poll().
			struct Pending
			{
				libusb_device *device;
				libusb_hotplug_event event;
			};


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


			/// Tranfer the given data to the device (write) or from the device (!write)
			bool Transfer(emg::Buffer<byte> *buffer, bool write, bool check, bool truncate);


			/// Releases the device.
			void Release();


			/// Product ID that we are interested in
			int product;

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

			/// A thread-safe queue for registering connection/disconnection events
			moodycamel::ConcurrentQueue<Pending> pending;

			// Windows does not yet support hotplugging so adapt accordingly
			bool legacy = false;


			/// Allows an internal global function to push onto the pending queue
			friend int LIBUSB_CALL OnHotplug(libusb_context *context, libusb_device *device, libusb_hotplug_event event, void *data);
	};
}
