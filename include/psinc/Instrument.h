#pragma once

#include <psinc/Transport.h>
#include <psinc/driver/Device.h>
#include <condition_variable>
#include <atomic>
#include <thread>


namespace psinc
{
	enum class ResetLevel
	{
		Connection		= 0xff,	// Full hardware reset at the USB transport level
		Control			= 0xfe,	// Full camera reset triggered via the USB control pipe
		Command			= 0x00,	// Full camera reset triggered via standard libpsinc command
		Communications	= 0x01,	// A soft reset that simply tells the camera to reset the communications buffer
		Imaging			= 0x02,	// A reset of the imaging chip only - registers will be refreshed automatically
		ImagingSoft		= 0x03,	// A soft reset of the imaging chip only - registers will be refreshed automatically
		Io				= 0x04	// Reset of the I/O sub-systems within the camera
	};


	/// The base class for a Camera but it can also be used directly when communicating
	/// with hardware that is not a camera or should you wish to use devices connected
	/// to a camera but not actually grab any images.
	class Instrument
	{
		public:

			enum class Type : uint16_t
			{
				Camera		= 0xaaca,
				Odometer	= 0xaac0,
			};

			struct Vendors
			{
				static const std::set<uint16_t> All;	// All vendors
				static const std::set<uint16_t> PSI;	// Only PSI
			};



			/// Default Constructor
			Instrument() {}

			/// Destructor
			virtual ~Instrument();

			/// Initialises the transport to look for specific descriptors or on a particular
			/// bus. It also starts the internal thread running. The serial string is actually
			/// treated as a regex and, combined with the cameras ability to append the camera
			/// name to the end of the serial number in the USB descriptor, provides a powerful
			/// way to reliably connect to a specific camera.
			virtual void Initialise(Type product, std::string serial = "", std::function<void(bool)> onConnection = nullptr, int timeout = 500, const std::set<uint16_t> &vendors = Vendors::All);

			/// Checks if this instance is currently connected to a physical device
			/// @return True if a device appears to be connected.
			virtual bool Connected();

			/// Create a custom device instance if you know the index instead of
			/// using the map of named devices below.
			Device CustomDevice(byte index);

			/// Resets the instrument.
			bool Reset(ResetLevel level = ResetLevel::Connection);


			/// Retrieve list of all serial numbers for any connected instruments of the given type
			static std::map<std::string, std::string> List(Type product = Type::Camera, const std::set<uint16_t> &vendors = Vendors::All);


			/// A map of available devices that can be controlled by (or are part of)
			/// the connected instrument.
			std::map<std::string, Device> devices;


		protected:

			/// Shuts down the thread, can be called by a derived class to ensure that
			/// members are not accessed after being deleted.
			void Dispose();

			/// Called when an instrument is connected. The instrument is not considered
			/// fully connected until it is successfully configured.
			virtual bool Configure() { return true; }

			/// Called from the thread main loop, it can be overridden to provide additional functionality
			/// Returns true if the thread is safe to go to sleep.
			virtual bool Main() { return true; }


			/// The communications layer, effectively a wrapper around libusb 1.0.
			Transport transport;

			/// Condition variable used to wake the thread
			std::condition_variable condition;

			/// Critical section mutex
			std::mutex cs;

			/// Invoked when the connection status changes
			std::function<void(bool)> onConnection = nullptr;


		private:

			/// Entry point for the thread
			void Entry();


			/// The thread
			std::thread _thread;

			/// Control flag for the thread
			std::atomic<bool> run;

			/// Set to true once initialised so that the thread is only created once
			/// even though the Initialise function can be called multiple times if the serial
			/// or instrument type needs to be modified.
			bool initialised = false;

			/// Set to true if the instrument is fully configured and refreshed after connection.
			bool configured = false;
	};
}
