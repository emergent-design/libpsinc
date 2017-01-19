#pragma once

#include <psinc/Transport.h>
#include <psinc/driver/Device.h>
#include <condition_variable>
#include <atomic>
#include <thread>


namespace psinc
{
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


			/// Default Constructor
			Instrument() {}

			/// Destructor
			virtual ~Instrument();

			/// Initialises the transport to look for specific descriptors or on a particular
			/// bus. It also starts the internal thread running. The serial string is actually
			/// treated as a regex and, combined with the cameras ability to append the camera
			/// name to the end of the serial number in the USB descriptor, provides a powerful
			/// way to reliably connect to a specific camera.
			virtual void Initialise(Type product, std::string serial = "", std::function<void(bool)> onConnection = nullptr, int timeout = 500);

			/// Checks if this instance is currently connected to a physical device
			/// @return True if a device appears to be connected.
			virtual bool Connected();

			/// Create a custom device instance if you know the index instead of
			/// using the map of named devices below.
			Device CustomDevice(byte index);

			/// Resets the instrument.
			///
			/// Level 0: full hardware reset at the USB transport level.
			/// Level 1: full soft reset at the camera chip level.
			/// Level 2: a soft reset that simply tells the camera to reset the communications buffers.
			///
			/// Level 2 is for use in the situation where an I/O error has caused the communications
			/// to potentially get out of step.
			bool Reset(byte level = 0);


			/// Retrieve list of all serial numbers for any connected instruments of the given type
			static std::map<std::string, std::string> List(Type product = Type::Camera);


			/// A map of available devices that can be controlled by (or are part of)
			/// the connected instrument.
			std::map<std::string, Device> devices;


		protected:

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
