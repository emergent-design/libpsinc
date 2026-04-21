#pragma once

#include <libusb-1.0/libusb.h>
#include <flasc/flasc.hpp>
#include <entity/entity.hpp>
#include <entity/json.hpp>
#include <emergent/FS.hpp>
#include <emergent/Io.hpp>


// Utilities for bootstrapping (initial flashing) a camera
class Bootstrap
{
	public:

		struct Configuration
		{
			string bootstrap;	// RAM-based bootstrap filename
			string updater;		// Updater firmware filename
			string bootloader;	// Bootloader firmware filename
			string application;	// Application firmware filename

			string serial;				// Updated with the serial of the device once it has been bootstrapped so that each subsequent stage can connect to the same device
			int timeout 	= 60'000;	// Timeout for the USB transfers

			string id		= "";	// Empty ID is not transmitted
			byte lighting	= 0x00;
			int watchdog	= 0;

			emap(
				eref(bootstrap), eref(updater), eref(bootloader), eref(application),
				eref(timeout), eref(id), eref(lighting), eref(watchdog)
			)
		};


		static bool Go(Flasc &flasc, Configuration &configuration)
		{
			Instrument instrument;

			return CheckFiles(flasc, configuration)
				&& flasc.RamFlash(configuration.bootstrap, true)
				&& WriteUpdater(flasc, instrument, configuration)
				&& WriteBootloader(flasc, instrument, configuration)
				&& WaitForDevice(flasc, configuration)
				&& flasc.Update(-1, "Writing application firmware")
				&& flasc.Flash(configuration.serial + ".*", configuration.application)
				&& ConfigureDevice(flasc, configuration)
				&& GenerateReport(flasc, configuration)
			;
		}


	private:

		static bool GenerateReport(Flasc &flasc, Configuration &configuration)
		{
			flasc.Update(0.0, "Generating report ..");

			std::ofstream file;
			emg::fs::path path	= "reports";
			auto data           = flasc.Version(configuration.serial + ".*");
			auto timestamp      = emg::Timestamp::Now();
			auto filename       = String::format("%s_%s_%s.log", emg::Timestamp::Date(), data["Type"], configuration.serial);

			emg::fs::create_directories(path);
			file.open((path / filename).string(), std::ios::out);

			if (!file.is_open())
			{
				return flasc.Abort(nullptr, "failed, error opening report file");
			}

			file
				<< "Device bootstrap report\n\n"
				<< "    Timestamp: " << timestamp << '\n'
				<< "         Type: " << data["Type"] << '\n'
				<< "       Serial: " << configuration.serial << '\n'
				<< "     Hardware: " << data["Hardware"] << '\n'
				<< "     Firmware: " << data["Firmware"] << '\n'
				<< "           ID: " << data["ID"] << '\n'
				<< "     Lighting: " << data["Lighting"] << '\n'
				<< "     Watchdog: " << data["Watchdog"] << '\n'
				<< "       Format: " << data["Format"] << '\n'
				<< "Configuration: "
				<< ent::encode<ent::prettyjson>(configuration)
				<< '\n'
			;

			file.close();

			flasc.Update(-1, "done");
			flasc.Update(0.0, "Updating device database ..");

			bool header = !emg::fs::exists(path / "devices.db");

			file.open((path / "devices.db").string(), std::ios::out | std::ios::app);

			if (header)
			{
				file << "Timestamp\tType\tSerial\tHardware\tFirmware\n";
			}

			file
				<< timestamp << '\t'
				<< data["Type"] << '\t'
				<< configuration.serial << '\t'
				<< data["Hardware"] << '\t'
				<< data["Firmware"] << '\n'
			;

			file.close();
			return flasc.Update(-1, "done");
		}


		static bool WaitForDevice(Flasc &flasc, const Configuration &configuration)
		{
			// Connect by serial number from this point on, to ensure we are dealing with the same camera
			const auto serial = configuration.serial + ".*";

			// flasc.Update(-1, "You now have 60s to disconnect and reconnect the camera ready for the final programming stages");
			flasc.Update(-1, "Waiting 60s for that camera to reappear ready for the final programming stages");

			for (int i=0; i<60; i++)
			{
				// Create a new instance each time - based on theory that libusb is blacklisting the camera
				// when it first appears after bootloader flashing due to it not responding sensibly yet.
				// By using a new instrument each time it will be a fresh context and hopefully a fresh
				// internal list of USB devices.
				Instrument instrument;
				instrument.Initialise(Instrument::Type::Camera, serial);

				if (flasc.ConnectWait(instrument, 100, 10))
				{
					return true;
				}
			}

			// If not successful then list any connected devices (Cypress and PSI)
			std::cout << "Listing 0xabcd:" << std::endl;
			for (auto &[k,v] : Instrument::List(0xabcd))
			{
				std::cout << "  " << k << ": " << v.description << std::endl;
			}

			std::cout << "Listing 0xaaca:" << std::endl;
			for (auto &[k,v] : Instrument::List())
			{
				std::cout << "  " << k << ": " << v.description << std::endl;
			}
			return false;
		}


		// static bool ConfigureDeviceAndWriteFirmware(Flasc &flasc, const Configuration &configuration)
		static bool ConfigureDevice(Flasc &flasc, const Configuration &configuration)
		{
			// Connect by serial number from this point on, to ensure we are dealing with the same camera
			const auto serial = configuration.serial + ".*";

			// Sleep between each step because it involves disconnecting and reconnecting rapidly
			std::this_thread::sleep_for(2s);

			if (!configuration.id.empty())
			{
				flasc.Update(-1, String::format("Setting device ID to '%s'", configuration.id));

				if (!flasc.SetId(serial, configuration.id))
				{
					flasc.Update(-1, "Failed to set ID");
				}

				// Sleep between each step because it involves disconnecting and reconnecting rapidly
				std::this_thread::sleep_for(2s);
			}

			flasc.Update(-1, String::format("Setting lighting type to '0x%02x'", configuration.lighting));
			if (!flasc.SetLighting(serial, configuration.lighting))
			{
				flasc.Update(-1, "Failed to set lighting type");
			}

			// Sleep between each step because it involves disconnecting and reconnecting rapidly
			std::this_thread::sleep_for(2s);

			flasc.Update(-1, String::format("Setting watchdog to '%d'", configuration.watchdog));
			if (!flasc.SetWatchdog(serial, configuration.watchdog))
			{
				flasc.Update(-1, "Failed to set watchdog");
			}

			// Sleep between each step because it involves disconnecting and reconnecting rapidly
			std::this_thread::sleep_for(2s);

			return true;
		}


		static bool WriteUpdater(Flasc &flasc, Instrument &instrument, Configuration &configuration)
		{
			std::vector<byte> updater;

			flasc.Update(0.0, "Loading updater program ..");

			if (!emg::Io::Load(updater, configuration.updater))
			{
				return flasc.Abort("failed to load updater");
			}

			flasc.Update(-1, "done");
			instrument.Initialise(0xabcd, "", nullptr, configuration.timeout, Instrument::Vendors::PSI);

			if (!flasc.ConnectWait(instrument, 1000, 30))
			{
				return false;
			}

			flasc.Update(0.0, "Retrieving serial number :");
			configuration.serial = Serial(instrument.devices["Serial"].Read());

			if (configuration.serial.empty())
			{
				return flasc.Abort("failed");
			}

			flasc.Update(-1, configuration.serial);
			flasc.Update(0.0, String::format("Writing updater program (%d bytes) .. ", updater.size()));

			instrument.CustomDevice(0x12).Write(updater);

			flasc.Update(-1, "done");
			flasc.Update(0.0, "Waiting to check device ..");

			for (int i=0; i<5; i++)
			{
				std::this_thread::sleep_for(1s);
				flasc.Update((double)i / 5.0);
			}

			auto query = instrument.CustomDevice(0xff).Read();

			if (query.size() == 0 || query[0] < 2 || query[1] == 0x00 || (query[2] & 0x80) == 0)
			{
				return flasc.Abort(String::format(
					"invalid, updater program failed to write, aborting (h%02x c%02x m%02x)",
					query.size() > 0 ? (int)query[0] : 0,
					query.size() > 1 ? (int)query[1] : 0,
					query.size() > 2 ? (int)query[2] : 0
				));
			}

			return flasc.Update(-1, "valid");
		}


		static bool WriteBootloader(Flasc &flasc, Instrument &instrument, const Configuration &configuration)
		{
			std::vector<byte> bootloader;

			flasc.Update(0.0, "Loading bootloader program ..");

			if (!emg::Io::Load(bootloader, configuration.bootloader))
			{
				return flasc.Abort("failed to load bootloader");
			}

			flasc.Update(-1, "done");
			flasc.Update(0.0, String::format("Writing bootloader program (%d bytes) ..", bootloader.size()));

			instrument.CustomDevice(0x12).Write(bootloader);

			flasc.Update(-1, "done");
			flasc.Update(0.0, "Allowing time for bootloader initialisation ");

			for (int i=0; i<5; i++)
			{
				std::this_thread::sleep_for(1s);
				flasc.Update((double)i / 5.0);
			}

			return flasc.Update(-1, "done");
		}


		static bool CheckFiles(Flasc &flasc, const Configuration &configuration)
		{
			flasc.Update(0.0, "Checking firmware files ..");

			if (!emg::fs::exists(configuration.bootstrap))
			{
				return flasc.Abort(String::format("bootstrap file '%s' does not exist", configuration.bootstrap));
			}

			if (!emg::fs::exists(configuration.updater))
			{
				return flasc.Abort(String::format("updater file '%s' does not exist", configuration.updater));
			}

			if (!emg::fs::exists(configuration.bootloader))
			{
				return flasc.Abort(String::format("bootloader file '%s' does not exist", configuration.bootloader));
			}

			if (!emg::fs::exists(configuration.application))
			{
				return flasc.Abort(String::format("application file '%s' does not exist", configuration.application));
			}

			return flasc.Update(-1, "done");
		}


		static string Serial(const std::vector<byte> &serial)
		{
			std::ostringstream result;

			result << std::hex << std::uppercase << std::setfill('0');

			for (auto b : serial)
			{
				result << std::setw(2) << (int)b;
			}

			return result.str();
		}
};

