#pragma once

#include <psinc/transport/TransportHotplug.h>
#include <psinc/driver/Device.h>
#include <condition_variable>
#include <atomic>
#include <thread>


namespace psinc
{
	class Instrument
	{
		public:

			enum class Type : int
			{
				Camera		= 0xaaca,
				Odometer	= 0xaac0,
			};


			/// Default Constructor
			Instrument();


			/// Destructor
			virtual ~Instrument();


			/// Initialises the transport to look for specific descriptors or on a particular
			/// bus. It also starts the internal thread running. The serial string is actually
			/// treated as a regex and, combined with the cameras ability to append the camera
			/// name to the end of the serial number in the USB descriptor, provides a powerful
			/// way to reliably connect to a specific camera.
			void Initialise(Type product, std::string serial = "", std::function<void(bool)> onConnection = nullptr);


			/// Checks if this instance is currently connected to a physical device
			/// @return True if a device appears to be connected.
			bool Connected();


			Device CustomDevice(byte index);

			/// Retrieve list of all serial numbers for any connected instruments of the given type
			static std::vector<std::string> List(Type product);

			/// A map of available devices that can be controlled by (or are part of)
			/// the connected instrument.
			std::map<std::string, Device> devices;


			/// Resets the instrument.
			bool Reset();


		private:

			/// Entry point for the thread
			virtual void Entry();



			/// The communications layer, effectively a wrapper around libusb 1.0.
			TransportHotplug transport;

			/// A map of all known devices per instrument.
			std::map<Type, std::map<byte, Device>> deviceSets;

			///Condition variable used to wake the thread when a grab request is made
			std::condition_variable condition;

			///Critical section mutex
			std::mutex cs;

			///The thread
			std::thread _thread;

			/// Control flag for the capture thread
			std::atomic<bool> run;

			/// Set to true once initialised so that the thread is only created once
			/// even though the Initialise function can be called multiple times if the serial
			/// and bus parameters need to be modified.
			bool initialised = false;

	};
}
