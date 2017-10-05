#pragma once

#include <psinc/Instrument.h>
#include <psinc/handlers/DataHandler.hpp>
#include <psinc/driver/Feature.h>
#include <psinc/driver/Aliases.h>
#include <psinc/driver/Device.h>
#include <psinc/Transport.h>

#include <thread>
#include <atomic>
#include <condition_variable>


namespace psinc
{
	/// The primary interface to the capture library
	class Camera : public Instrument
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
			virtual void Initialise(std::string serial = "", std::function<void(bool)> onConnection = nullptr, int timeout = 500, const std::set<uint16_t> &vendors = Vendors::All);

			/// Start an asynchronous image grab. The data handler would usually have access
			/// to an image and will be used to automatically convert the raw data from the
			/// camera. The callback event is invoked regardless of what happened and reports
			/// the AcquisitionStatus. If the callback function returns "true" then this Camera
			/// will go ahead and capture another frame (streaming mode), and if "false" is
			/// returned then it will stop and await the next call to GrabImage.
			/// @return False if grabbing could not be started (already grabbing)
			bool GrabImage(Mode mode, DataHandler &handler, std::function<bool(bool)> callback);

			/// Checks if the camera is currently performing an asynchronous image grab.
			/// @return True if the camera is currently grabbing.
			bool Grabbing();

			/// Sets the flash power. Depending on the camera type this can be
			/// a xenon or LED flash.
			void SetFlash(byte power);

			/// Change the context for camera chips that support multiple contexts.
			/// Multiple contexts allow sets of features to be configured and rapidly
			/// switched between.
			bool SetContext(byte context);

			/// Helper function to configure which part of the sensor to use
			/// for image capture regardless of chip type (since chips define
			/// regions in different ways). Width or height of -1 will use
			/// the default value instead.
			bool SetWindow(byte context, int x = 0, int y = 0, int width = -1, int height = -1);

			// WARNING: Expert-only commands for direct access to registers. Using these
			// will render the values stored in the feature maps above obsolete. Setting
			// invalid values could leave the chip in an inoperative state.
			bool SetRegister(int address, int value);

			// Will return -1 if the value could not be retrieved.
			int GetRegister(int address);

			// Returns the chip type for the currently connected camera or "unknown" if no camera is present.
			const std::string GetType();

			/// A map of feature name to Feature instance for the connected device
			/// A "Feature" is a representation of a property of the imaging chip.
			std::map<std::string, Feature> features;

			/// Maps common features to simple names grouped by context.
			std::map<byte, Aliases> aliases;


		protected:

			/// When a camera is connected, determine what type it is and then initialise
			/// the lists of available features and aliases.
			virtual bool Configure();

			// Called from the thread main loop and returns true if the thread is safe to go to sleep.
			virtual bool Main();


		private:

			// Disallow the use of the base class initialisation since we need to override
			// some of the connection functionality - a camera is not considered fully
			// connected until it has also been configured.
			using Instrument::Initialise;

			/// The internal image handler created for the specific type of image passed to
			/// the grab function.
			DataHandler *handler = nullptr;

			/// Initialise the features and registers from a chip specific XML document.
			bool Configure(pugi::xml_node xml);

			/// Request all register values from the camera to synchronise it with those
			/// accessible in this driver.
			bool RefreshRegisters();

			/// Attempt to capture data from the device. The supplied handler should
			/// be of the appropriate type to cope with the data that will be captured.
			/// @return AcquisitionStatus
			bool Capture(DataHandler *handler, Mode mode, int flash);


			/// The chip registers for the connected camera. The map key is the register
			/// address.
			std::map<int, Register> registers;

			/// Byte buffer containing the packet to be transmitted to the actual device
			/// this class represents.
			emg::Buffer<byte> send;

			/// Byte buffer used to receive data from the device during a capture.
			emg::Buffer<byte> receive;


			/// Image capture complete callback
			std::function<bool(bool)> callback	= nullptr;

			/// Storage for the capture mode
			Mode mode = Mode::Normal;

			/// The PSI camera range can contain either bayer or monochrome chips. Monochrome
			/// is the default but this is set to false if the chip is determined to be bayer
			/// on connection. This flag then indicates which conversion functions should be
			/// used in the data handler.
			bool monochrome = true;

			/// Storage for the flash power value
			byte flash = 0;

			/// The current camera context (where appropriate)
			byte context = 0;

			/// The maximum number of contexts for the current chip type.
			byte contextCount = 1;

			// Size of the address location for a register. This can vary by imaging chip type.
			byte addressSize = 1;

			/// Indicates that this is an HDR camera (dealing with 16-bit data instead of 8)
			bool hdr = false;

			/// Indicates an offset in the bayer grid since different chips start at different positions.
			byte bayerMode = 0;

			// Informs the capture function which method to use to determine the amount of image
			// data to expect. In normal operation it looks for "Width" and "Height" aliases, whereas
			// in sizeByRange mode it expects to find "ColumnStart", "RowStart", "ColumnEnd", "RowEnd".
			bool sizeByRange = false;

			// The current chip type
			std::string chip = "unknown";

			// A lock used for ensuring that changes to the window do not occur whilst grabbing an image
			std::mutex window;
	};
}
