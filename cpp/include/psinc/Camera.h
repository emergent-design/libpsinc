#pragma once

#include <psinc/transport/Transport.h>
#include <psinc/handlers/DataHandler.h>
#include <psinc/driver/Feature.h>
#include <psinc/driver/Device.h>

#include <thread>
#include <condition_variable>


namespace psinc
{
	enum AcquisitionStatus
	{
		ACQUISITION_SUCCESSFUL,
		ACQUISITION_CONNECTED,
		ACQUISITION_DISCONNECTED,

		ACQUISITION_ERROR_TRANSFER_FAILED			= -1,
		ACQUISITION_ERROR_IMAGE_CONVERSION_FAILED	= -2,
	};


	/// The primary interface to the capture library
	class Camera
	{
		public:
		
			enum class Mode
			{
				Normal,
				Master,
				SlaveRising,
				SlaveFalling
			};

			/// Default Constructor
			Camera();

			///Destructor
			virtual ~Camera();
			
			
			void Initialise(std::string serial = "", int bus = 0);

			/// Start an asynchronous image grab. The supplied image will be automatically
			/// resized to the appropriate width and height by the underlying handler.
			/// @param[out] handler An appropriate data handler.
			/// @param[in] callback Method to be invoked on completion of the grab.
			/// @return False if grabbing could not be started (already grabbing)
			bool GrabImage(Mode mode, DataHandler &handler, emg::event callback);

			/// Checks if this instance is currently connected to a physical device
			/// @return True if a device appears to be connected.
			bool Connected();

			/// Checks if the camera is currently performing an asynchronous image grab.
			/// @return True if the camera is currently grabbing.
			bool Grabbing();

			/// A map of feature name to Feature instance for the connected device
			/// (A "Feature" is a code representation of a property of the imaging chip,
			/// or other connected device such as a flash).
			//FeatureMap features;
			std::map<std::string, Feature> features;
			
			
			// Maps common features to simple names
			//AliasMap aliases;
			std::map<byte, std::map<std::string, Feature*>> aliases;
			
			//DeviceMap devices;
			std::map<std::string, Device> devices;

			/// Initialises the camera
			/// @param[in] xml The device configuration XML
			/// @param[in] transport The type of transport required
			//bool Initialise(Acquire::Type type, std::string xml, std::string serial = "", int bus = 0, Transport::Type transport = Transport::Type::USB);			
			//bool Initialise(std::string type, std::string chip, std::string serial = "", int bus = 0, Transport::Type transport = Transport::Type::USB);


			void SetFlash(int power);
			//void SetSlave(bool enabled);
			bool SetContext(byte context);
			
			/// Resets the camera.
			bool Reset();

		protected:

			/// Representation of the physical device currently connected to this Camera.
			/// null if disconnected.
			//Camera *camera = nullptr;

			/// The internal image handler created for the specific type of image passed to
			/// the grab function.
			DataHandler *handler = nullptr;

		private:

			///Entry point for the asynchronous grabbing thread
			virtual void Entry();

			bool Connect();
			
			bool Configure();
			bool Configure(pugi::xml_node xml);
			bool RefreshRegisters();
			
			/// Attempt to capture data from the device. The supplied handler should
			/// be of the appropriate type to cope with the data that will be captured.
			/// @return Acquisition error code
			int Capture(DataHandler *image, Mode mode, int flash);
			
			Transport transport;
			
			std::map<byte, Device> devicePool;
			
			std::map<int, Register> registers;
			
			/// Byte buffer containing the packet to be transmitted to the actual device this class represents.
			emg::Buffer<byte> send;

			/// Byte buffer used to receive data from the device during a capture.
			emg::Buffer<byte> receive;

			///Condition variable used to wake the thread when a grab request is made
			std::condition_variable condition;

			///Image capture complete callback
			emg::event callback	= nullptr;

			///Critical section mutex
			std::mutex cs;

			///The capture thread
			std::thread _thread;

			Mode mode			= Mode::Normal;
			volatile bool exit			= false;
			bool monochrome		= false;
			bool initialised	= false;
			int flash			= 0;
			
			byte context		= 0;
			byte contextCount	= 1;
	};
}