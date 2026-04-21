#pragma once

#include <emergent/logger/Logger.hpp>
#include <emergent/Uuid.hpp>
#include <emergent/Io.hpp>
#include <psinc/Instrument.h>
#include <iomanip>

using namespace std::chrono;
using namespace psinc;

using std::string;
using emg::byte;
using emg::String;


class Flasc
{
	public:

		Flasc() {}
		Flasc(std::function<void(double, string)> callback, int timeout = 500) : callback(callback), timeout(timeout) {}


		void SetCallback(std::function<void(double, string)> callback)
		{
			this->callback = callback;
		}


		static inline string to_string(const std::vector<byte> &data)
		{
			return { data.begin(), data.end() };
		}


		std::map<string, string> Version(string serial)
		{
			Instrument instrument;
			std::map<string, string> result;

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				auto query = instrument.devices["Query"].Read();

				if (query.size() && query[0] > 1)
				{
					result["Type"]		= TYPE_TABLE[query[1]];
					result["Format"]	= query[2] & 0x01 ? "bayer" : "monochrome";
					result["Mode"]		= query[2] & 0x80 ? "flash" : "normal";
				}
				else result["Type"]	= TYPE_TABLE[0];

				result["Serial"]	= ToString(instrument.devices["Serial"].Read());
				result["ID"]		= String::trim(to_string(instrument.devices["Name"].Read()), ':');
				result["Hardware"]	= String::trim(to_string(instrument.CustomDevice(0x0b).Read()), '\0');
				result["Firmware"] 	= String::trim(to_string(instrument.CustomDevice(0x0f).Read()), '\0');
				result["Watchdog"]	= std::to_string(ToInt(instrument.CustomDevice(0x0d).Read()));

				if (result["Mode"] == "normal")
				{
					byte lighting		= instrument.CustomDevice(0x0c).Read()[0];
					result["Lighting"]	= String::format("%d (%s)", lighting, LIGHTING_TABLE[lighting]);
				}
			}

			return result;
		}


		// Flash application directly to RAM on the camera - requires the camera
		// to be in the appropriate mode where it will appear as a Cypress device.
		// If bootstrap is specified then the offset is different because we are
		// writing the bootstrap program to the camera during initial flashing.
		bool RamFlash(const string &path, bool bootstrap)
		{
			this->Update(0.0, "Looking for Cypress boot device ..");
			libusb_device_handle *handle = nullptr;

			for (int i=0; i<20 && !handle; i++)
			{
				handle = libusb_open_device_with_vid_pid(nullptr, 0x04b4, 0x00f3);

				if (!handle)
				{
					this->Update((double)(i + 1) / 20);
					std::this_thread::sleep_for(1s);
				}
			}

			if (!handle)
			{
				this->Update(-1, "not found, aborting");
				return false;
			}

			this->Update(-1, "found");
			this->Update(0.0, "Claiming device ..");

			if (libusb_claim_interface(handle, 0) != 0)
			{
				return this->Abort(handle, "failed");
			}

			// Vendor specific request, returns information on the device type - do we need to do this?
			uint8_t scratch[8];
			libusb_control_transfer(handle, 0xc0, 0xb0, 0, 0, scratch, sizeof(scratch), 5000);

			std::vector<uint8_t> buffer;
			this->Update(0.0, "Loading bootstrap program ..");

			if (!emg::Io::Load(buffer, path))
			{
				return this->Abort(handle, "failed to load bootstrap program, aborting");
			}

			int offset			= bootstrap ? 4 : 10;
			uint8_t *data		= buffer.data();
			const int length	= buffer.size();

			if (length < offset || data[offset - 4] != 'C' || data[offset - 3] != 'Y')
			{
				return this->Abort(handle, "invalid");
			}

			this->Update(-1, "done");
			this->Update(0.0, "Writing bootstrap program ");

			uint32_t address = 0;

			while (offset < length)
			{
				int sector = (length - offset) >= 4 ? *reinterpret_cast<uint32_t *>(data + offset) * 4 : 0;
				offset += 4;

				if (sector > (length - offset))
				{
					return this->Abort(handle, "error, invalid sector length in firmware file");
				}

				address = *reinterpret_cast<uint32_t *>(data + offset);
				offset += 4;

				// Last sector size before file checksum is 0
				if (sector == 0)
				{
					break;
				}

				while (sector > 0)
				{
					auto size	= std::min(2048, sector);
					int status	= libusb_control_transfer(handle, 0x40, 0xa0, (uint16_t)address, (uint16_t)(address >> 16), data + offset, (uint16_t)size, 5000);

					if (status < 0)
					{
						return this->Abort(handle, String::format("error writing to device (%d)", status));
					}
					if (status != size)
					{
						return this->Abort(handle, String::format("error, expected %d bytes written but only %d bytes were", size, status));
					}

					address	+= size;
					offset	+= size;
					sector	-= size;

					this->Update((double)offset / (double)length);
				}
			}

			// Send termination to initiate program load
			libusb_control_transfer(handle, 0x40, 0xa0, (uint16_t)address, (uint16_t)(address >> 16), nullptr, 0, 5000);
			libusb_close(handle);

			std::this_thread::sleep_for(500ms);

			return this->Update(-1, "done");
		}


		bool Flash(string serial, string path)
		{
			Instrument instrument;
			std::vector<byte> reference;
			bool legacy = true;

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				reference	= instrument.devices["Serial"].Read();
				auto query	= instrument.devices["Query"].Read();
				legacy		= !query.size() || query[0] < 2 || query[1] == 0x00;	// Assume legacy if query is empty

				if (!legacy && query.size() && query[0] > 1 && (query[2] & 0x80))
				{
					this->Update(-1, "Camera already in flash mode");
				}
				else
				{
					this->Update(-1, "Switching to flash mode");
					instrument.CustomDevice(0x12).Initialise(0x00);
					std::this_thread::sleep_for(milliseconds(1000));
				}
			}
			else this->Update(-1, "Checking if any device is waiting in flash mode (warning this may not be the device you expect if there are multiple devices in flash mode on this system)");

			auto firmware = this->LoadFirmware(path, legacy);

			if (firmware.size())
			{
				return legacy ? this->FlashLegacy(instrument, reference, firmware) : this->Flash(instrument, firmware);
			}

			return !this->Update(-1, "Firmware file not found or specified, aborting");
		}


		// Toggle update mode
		bool Toggle(string serial)
		{
			Instrument instrument;

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				this->Update(-1, "Toggling flash mode");

				instrument.CustomDevice(0x12).Initialise(0x00);
				std::this_thread::sleep_for(milliseconds(1000));

				if (this->ConnectWait(instrument, 1000, 30))
				{
					auto query = instrument.devices["Query"].Read();

					if (query.size() && query[0] > 1 && query[1] > 0x00)
					{
						return this->Update(-1, String::format("Success, device is now in %s mode", query[2] & 0x80 ? "flash" : "normal"));
					}
				}

				this->Update(-1, "Timed out, this may be a legacy device");
			}
			return false;
		}


		// Kick a legacy (v024) camera out of flash mode
		bool Kick(string serial)
		{
			Instrument instrument;
			Transport transport;

			if (this->LegacyConnect(transport))
			{
				std::vector<byte> fake = { 0 };

				this->Update(0.0, "Kicking out of flash mode .. ");
				this->HackedWrite(transport, fake, 0x12);
				this->Update(-1, "done");

				if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 1000, 30))
				{
					return this->Update(-1, "Device reconnected, kicking was successful");
				}
				else this->Update(-1, "Timed out waiting for device, something may have gone wrong!");
			}

			return false;
		}


		std::map<int, int> LoadDefaults(string path)
		{
			std::map<int, int> result;

			for (auto &line : String::explode(String::load(path), "\n"))
			{
				auto pair = String::explode(line, "=");

				if (pair.size() == 2)
				{
					result[strtol(pair[0].c_str(), nullptr, 0)] = strtol(pair[1].c_str(), nullptr, 0);
				}
			}

			return result;
		}


		bool ResetDefaults(string serial)
		{
			Instrument instrument;

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				if (instrument.CustomDevice(0x09).Initialise(0x00))
				{
					return this->Update(-1, "Success");
				}
				else this->Update(-1, "Failed");
			}

			return false;
		}


		bool SetDefaults(string serial, std::map<int, int> values)
		{
			Instrument instrument;

			std::vector<byte> data(3 * values.size());
			byte *d = data.data();

			for (auto &v : values)
			{
				*d++ = (byte)v.first;
				*d++ = (byte)(v.second & 0xff);
				*d++ = (byte)((v.second >> 8) & 0xff);
			}

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				if (instrument.CustomDevice(0x09).Write(data))
				{
					return this->Update(-1, "Success");
				}
				else this->Update(-1, "Failed");
			}

			return false;
		}


		bool SetSerial()
		{
			Instrument instrument;

			if (this->ConnectWait(instrument, Instrument::Type::Camera, "", 100, 50))
			{
				if (instrument.devices["Serial"].Read().size() == 0)
				{
					auto data	= emg::uuid().to_binary();
					int size	= data.size();
					std::vector<byte> serial(size);

					for (int i=0; i<size; i++) serial[i] = data[i];

					if (instrument.devices["Serial"].Write(serial))
					{
						return this->Update(-1, "Success");
					}
					else this->Update(-1, "Failed");
				}
				else
				{
					this->Update(-1, "This device has already been initialised with a serial number");
					this->Update(-1, "If there are multiple devices plugged into this machine please remove all except the uninitialised one");
				}
			}

			return false;
		}


		bool EnableSerial(string serial)
		{
			Instrument instrument;

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				if (instrument.devices["Serial"].Initialise(0x00))
				{
					return this->Update(-1, "Success");
				}
				else return this->Update(-1, "Failed");
			}

			return false;
		}



		bool SetId(string serial, string id)
		{
			if (!id.empty() && id[0] != ':') id = ':' + id;

			Instrument instrument;
			std::vector<byte> data = { id.begin(), id.end() };

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				if (instrument.CustomDevice(0x07).Write(data))
				{
					return this->Update(-1, "Success");
				}
				else return this->Update(-1, "Failed");
			}

			return false;
		}



		bool SetLighting(string serial, byte lighting)
		{
			if (!LIGHTING_TABLE.count(lighting)) return false;

			Instrument instrument;
			std::vector<byte> data = { lighting };

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))
			{
				if (instrument.CustomDevice(0x0c).Write(data))
				{
					return this->Update(-1, "Success");
				}
				else return this->Update(-1, "Failed");
			}

			return false;
		}


		bool SetWatchdog(string serial, uint32_t watchdog)
		{
			Instrument instrument;
			std::vector<byte> data(reinterpret_cast<byte *>(&watchdog), reinterpret_cast<byte *>(&watchdog) + 4);

			if (this->ConnectWait(instrument, Instrument::Type::Camera, serial, 100, 50))

			{
				if (instrument.CustomDevice(0x0d).Write(data))
				{
					return this->Update(-1, "Success");
				}
				else return this->Update(-1, "Failed");
			}

			return false;
		}

		// If not connected, wait for a timeout
		bool ConnectWait(Instrument &instrument, Instrument::Type product, string serial, int time, int count)
		{
			instrument.Initialise(product, serial, nullptr, this->timeout);

			return this->ConnectWait(instrument, time, count);
		}


		bool ConnectWait(Instrument &instrument, int time, int count)
		{
			bool result = instrument.Connected();

			this->Update(0.0, "Waiting for device  ");

			if (!result)
			{
				for (int i=0; i<count && !result; i++)
				{
					this->Update((double)(i+1) / (double)count);

					std::this_thread::sleep_for(milliseconds(time));

					result = instrument.Connected();
				}
			}

			this->Update(-1, result ? "found" : "not found, aborting");

			return result;
		}


		// This is public so that it can also be used by the bootstrap helper
		bool Abort(libusb_device_handle *handle, string message)
		{
			if (handle)
			{
				libusb_close(handle);
			}

			return this->Abort(message);
		}


		bool Abort(const string &message)
		{
			return !this->Update(-1, message + ", aborting");
		}


		// This is public so that it can also be used by the bootstrap helper
		bool Update(double progress, const string &message = "")
		{
			if (this->callback)
			{
				this->callback(progress, message);
			}

			return true;
		}


	private:

		static string ToString(const std::vector<byte> &b)
		{
			std::ostringstream result;

			result << std::hex << std::setfill('0');

			for (auto i : b)
			{
				result << std::setw(2) << (int)i << " ";
			}

			return result.str();
		}


		static uint32_t ToInt(const std::vector<byte> &b)
		{
			return b.size() == 4 ? *reinterpret_cast<const uint32_t *>(b.data()) : 0;
		}


		// Strip out carriage returns (silly Windows)
		std::vector<byte> LoadFirmware(string path, bool legacy)
		{
			auto firmware = String::load(path);

			if (legacy)
			{
				string result;
				result.reserve(firmware.size());
				for (auto &i : firmware)
				{
					if (i != '\r') result += i;
				}

				return { result.begin(), result.end() };
			}

			return { firmware.begin(), firmware.end() };
		}


		bool Flash(Instrument &instrument, const std::vector<byte> &firmware)
		{
			if (this->ConnectWait(instrument, 1000, 30))
			{
				auto query	= instrument.devices["Query"].Read();

				if (query.size() && query[0] > 1 && query[1] > 0x00)
				{
					if (query[2] & 0x80)
					{
						this->Update(0.0, String::format("Writing firmware (%d bytes)  ", firmware.size()));

						instrument.SetTimeout(FLASH_TIMEOUT);
						instrument.CustomDevice(0x12).Write(firmware);
						instrument.SetTimeout(this->timeout);

						this->Update(-1, "done");
						this->Wait(5);

						if (this->ConnectWait(instrument, 1000, 30))
						{
							return this->Update(-1, "Device reconnected, flashing was successful");
						}
						else
						{
							this->Update(-1, "Timed out waiting for device, something may have gone wrong!");
						}
					}
					else this->Update(-1, "Device is not in update mode, aborting");
				}
				else this->Update(-1, "Incorrect device type for this flash mode");
			}

			return false;
		}


		bool FlashLegacy(Instrument &instrument, const std::vector<byte> &reference, const std::vector <byte> &firmware)
		{
			std::vector<byte> buffer;
			Transport transport;
			std::atomic<bool> waiting(false);

			if (this->LegacyConnect(transport))
			{
				if (this->LegacyVerify(transport, reference))
				{
					int size = firmware.size();

					this->Update(0.0, String::format("Writing firmware (%d bytes)  ", size));

					for (int i=0; i<size; i+=BLOCKSIZE)
					{
						if (i % (4 * BLOCKSIZE) == 0) this->Update((double)i / (double)size);

						transport.Poll(0);
						// buffer.Set(firmware.data() + i, std::min(BLOCKSIZE, size - i));
						buffer.assign(
							firmware.data() + i,
							firmware.data() + i + std::min(BLOCKSIZE, size - i)
						);

						if (!transport.Transfer(&buffer, nullptr, waiting))
						{
							return !this->Update(-1, "failed");
						}
					}

					this->Update(-1, "done");

					if (this->ConnectWait(instrument, 1000, 30))
					{
						return this->Update(-1, "Device reconnected, flashing was successful");
					}
					else
					{
						this->Update(-1, "Timed out waiting for device, something may have gone wrong!");
					}
				}
			}

			return false;
		}


		void Wait(int count)
		{
			this->Update(0.0, String::format("Waiting for %d seconds  ", count));

			for (int i=0; i<count; i++)
			{
				std::this_thread::sleep_for(1s);
				this->Update((double)(i + 1) / (double)count);
			}

			this->Update(-1, "done");
		}


		bool LegacyConnect(Transport &transport)
		{
			bool result = false;

			transport.Initialise(Instrument::Vendors::All, FLASHMODE, "", nullptr, 2000);

			this->Update(0.0, "Waiting for device  ");

			for (int i=0; i<30 && !result; i++)
			{
				this->Update((double)i / 30.0);
				std::this_thread::sleep_for(milliseconds(1000));

				transport.Poll(0);

				result = transport.Connected();
			}

			this->Update(-1, result ? "found" : "not found, aborting");

			return result;
		}


		bool LegacyVerify(Transport &transport, const std::vector<byte> &reference)
		{
			auto serial			= Device(&transport, "", 0x05).Read();	std::this_thread::sleep_for(10ms);
			auto validation 	= Device(&transport, "", 0x0b).Read();	std::this_thread::sleep_for(10ms);
			bool fail			= false;
			size_t size			= validation.size();

			if (!size)
			{
				this->Update(-1, "Error reading hardware version");
				return false;
			}

			if (reference.size())
			{
				if (reference.size() != serial.size())				fail = true;
				for (size_t i=0; i<reference.size() && !fail; i++)	fail = reference[i] != serial[i];

				if (fail)
				{
					this->Update(-1, "Serial number mismatch, aborting");
					return false;
				}
			}

			// Hack since the hardware string has a stray null at the end
			if (validation[size - 1] == 0)
			{
				// validation.Truncate(--size);
				validation.resize(--size);
			}

			if (serial.size() >= size)
			{
				// If there is a serial number then generate the validation message accordingly.
				// The only time there should be no serial number is when it is first being flashed
				// by the manufacturer. In this case the validation value can simply be the hardware
				// version.
				for (size_t i=0; i<size; i++) validation[i] = (byte)(validation[i] ^ serial[i]);
			}

			this->Update(0.0, "Initialising device .. ");

			if (!this->HackedWrite(transport, validation, 0x12))
			{
				this->Update(-1, "failed");
				return false;
			}

			this->Update(-1, "done");

			return true;
		}


		// Instead of device write since the bootloader has a bug
		// meaning it is expecting the wrong command byte.
		bool HackedWrite(Transport &transport, const std::vector<byte> &buffer, byte index)
		{
			std::atomic<bool> waiting(false);

			int size		= buffer.size();
			byte command[]	= {
				0x00, 0x00, 0x00, 0x00, 0x00, 		// Header
				0x20, index, 						// Command (should be 0x23)
				(byte)(size & 0xff), 				// Size
				(byte)((size >> 8) & 0xff),
				(byte)((size >> 16) & 0xff),
			};

			std::vector<byte> data(11 + size);

			memcpy(data.data(), command, 10);
			memcpy(data.data() + 10, buffer.data(), size);
			data[size+10] = 0xff;					// Terminator

			return transport.Transfer(&data, nullptr, waiting);
		}


		const int BLOCKSIZE		= 512;
		const int FLASHMODE		= 0xabca;
		const int FLASH_TIMEOUT	= 60000;

		std::function<void(double, string)> callback	= nullptr;
		int timeout										= 500;

		std::map<byte, string> LIGHTING_TABLE = {
			{ 0x00, "" },
			{ 0x01, "xenon" },
			{ 0x02, "legacy-led" },
			{ 0x03, "psiloc-led" },
			{ 0x04, "hpfc" }
		};

		std::map<byte, string> TYPE_TABLE = {
			{ 0x00, "v024" },
			{ 0x01, "mt9" }
		};
};

