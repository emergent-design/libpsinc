#pragma once

#include <psinc/transport/Transport.h>
#include <psinc/handlers/DataHandler.h>
#include <psinc/driver/Feature.h>
#include <psinc/driver/Device.h>

#include <thread>
#include <condition_variable>


namespace psinc
{
	/// Capture status values passed to the event callback function
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

			/// The PSI cameras have different modes of capture
			/// which can be set during a call to GrabImage
			enum class Mode
			{
				Normal,			///< Default capture mode
				Master,			///< Capture as master, will trigger slaves
				SlaveRising,	///< Capture as a slave on a rising edge of the trigger signal
				SlaveFalling	///< Capture as a slave on the falling edge of the trigger signal
			};


			/// Default Constructor
			Camera();


			/// Destructor
			virtual ~Camera();


			/// Initialises the transport to look for specific descriptors or on a particular
			/// bus. It also starts the internal thread running. The serial string is actually
			/// treated as a regex and, combined with the cameras ability to append the camera
			/// name to the end of the serial number in the USB descriptor, provides a powerful
			/// way to reliably connect to a specific camera.
			void Initialise(std::string serial = "", int bus = 0);


			/// Start an asynchronous image grab. The data handler would usually have access
			/// to an image and will be used to automatically convert the raw data from the
			/// camera. The callback event is invoked regardless of what happened and reports
			/// the AcquisitionStatus. If the callback function returns "true" then this Camera
			/// will go ahead and capture another frame (streaming mode), and if "false" is
			/// returned then it will stop and await the next call to GrabImage.
			/// @return False if grabbing could not be started (already grabbing)
			bool GrabImage(Mode mode, DataHandler &handler, emg::event callback);


			/// Checks if this instance is currently connected to a physical device
			/// @return True if a device appears to be connected.
			bool Connected();


			/// Checks if the camera is currently performing an asynchronous image grab.
			/// @return True if the camera is currently grabbing.
			bool Grabbing();


			/// A map of feature name to Feature instance for the connected device
			/// A "Feature" is a representation of a property of the imaging chip.
			std::map<std::string, Feature> features;


			/// Maps common features to simple names
			std::map<byte, std::map<std::string, Feature*>> aliases;


			/// A map of available devices that can be controlled by (or are part of)
			/// the connected camera.
			std::map<std::string, Device> devices;


			/// Sets the flash power. Depending on the camera type this can be
			/// a xenon or LED flash.
			void SetFlash(byte power);


			/// Change the context for camera chips that support multiple contexts.
			/// Multiple contexts allow sets of features to be configured and rapidly
			/// switched between.
			bool SetContext(byte context);


			/// Resets the camera.
			bool Reset();


		private:

			/// The internal image handler created for the specific type of image passed to
			/// the grab function.
			DataHandler *handler = nullptr;


			/// Entry point for the asynchronous grabbing thread
			virtual void Entry();

			/// Informs the underlying transport to connect to a device and if successful
			/// will invoke configuration and refreshing of the register values.
			bool Connect();


			/// When a camera is connected, determine what type it is and then initialise
			/// the lists of available features, aliases and devices.
			bool Configure();


			/// Initialise the features and registers from a chip specific XML document.
			bool Configure(pugi::xml_node xml);


			/// Request all register values from the camera to synchronise it with those
			/// accessible in this driver.
			bool RefreshRegisters();


			/// Attempt to capture data from the device. The supplied handler should
			/// be of the appropriate type to cope with the data that will be captured.
			/// @return AcquisitionStatus
			int Capture(DataHandler *image, Mode mode, int flash);


			/// The communications layer, effectively a wrapper around libusb 1.0.
			Transport transport;

			/// A map of all known devices.
			std::map<byte, Device> devicePool;

			/// The chip registers for the connected camera. The map key is the register
			/// address.
			std::map<int, Register> registers;

			/// Byte buffer containing the packet to be transmitted to the actual device
			/// this class represents.
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

			/// Storage for the capture mode
			Mode mode = Mode::Normal;

			/// Control flag for the capture thread
			volatile bool exit = false;

			/// The PSI camera range can contain either bayer or monochrome chips. Monochrome
			/// is the default but this is set to false if the chip is determined to be bayer
			/// on connection. This flag then indicates which conversion functions should be
			/// used in the data handler.
			bool monochrome = true;

			/// Set to true once initialised so that the capture thread is only created once
			/// even though the Initialise function can be called multiple times if the serial
			/// and bus parameters need to be modified.
			bool initialised = false;

			/// Storage for the flash power value
			byte flash = 0;

			/// The current camera context (where appropriate)
			byte context = 0;

			/// The maximum number of contexts for the current chip type.
			byte contextCount = 1;
	};
}
