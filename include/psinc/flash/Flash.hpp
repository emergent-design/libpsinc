#pragma once

#include <string>
#include <regex>
#include <emergent/String.hpp>
#include <emergent/Timer.hpp>
#include <emergent/logger/Logger.hpp>
#include <psinc/flash/HPFC.hpp>
#include <libserialport.h>
#include <emergent/FS.hpp>


namespace psinc::flash
{
	namespace fs = emg::fs;
	using namespace std::chrono;

	// A helper class for communicating with PSI flash driver units over RS485. A binary that uses this header
	// must link with libserialport.
	class Flash
	{
		public:

			struct Status
			{
				long write		= 0;	// Number of writes
				long read		= 0;	// Number of reads
				long crc		= 0;	// Number of CRC errors during read
				long errors		= 0;	// Number of read/write errors
			};

			template <size_t N> using Result = std::tuple<bool, std::array<uint16_t, N>>;


			~Flash()
			{
				this->Free();
			}


			// Set the serial port connection string and device address, then initiate a connection
			bool Initialise(const std::string &connection)
			{
				this->connection = connection;
				return this->Connect();
			}


			// Helper function to find the connection string for a specific USB based serial port.
			// It accepts the same type of regex serial string as a Camera/Instrument.
			static std::string Find(const std::string &serial)
			{
				sp_port **list;
				std::string result = "";

				if (sp_list_ports(&list) == SP_OK)
				{
					for (sp_port **port = list; *port; port++)
					{
						if (sp_get_port_transport(*port) == SP_TRANSPORT_USB)
						{
							auto name = sp_get_port_usb_serial(*port);
							if (name)
							{
								auto id = emg::String::trim(name, ' ');

								if (std::regex_match(id, std::regex(serial)))
								{
									result = sp_get_port_name(*port);
									break;
								}
							}
						}
					}

					sp_free_port_list(list);
				}

				return result;
			}


			// List all serial ports on the system
			static std::map<std::string, std::string> List()
			{
				sp_port **list;
				std::map<std::string, std::string> result;

				if (sp_list_ports(&list) == SP_OK)
				{
					for (sp_port **port = list; *port; port++)
					{
						result[sp_get_port_name(*port)] = sp_get_port_description(*port);
						// std::cout << sp_get_port_name(*port) << std::endl;
						// std::cout << "\t Description: " << sp_get_port_description(*port) << std::endl;

						// if (sp_get_port_transport(*port) == SP_TRANSPORT_USB)
						// {
						// 	// Test code only for the moment
						// 	int vid=0, pid=0, bus=0, address=0;
						// 	sp_get_port_usb_vid_pid(*port, &vid, &pid);
						// 	sp_get_port_usb_bus_address(*port, &bus, &address);

						// 	std::cout << emg::String::format("\t         VID: 0x%04x", vid) << std::endl;
						// 	std::cout << emg::String::format("\t         PID: 0x%04x", pid) << std::endl;
						// 	std::cout << emg::String::format("\t         Bus: %d", bus) << std::endl;
						// 	std::cout << emg::String::format("\t     Address: %d", address) << std::endl;

						// 	auto man		= sp_get_port_usb_manufacturer(*port);
						// 	auto product	= sp_get_port_usb_product(*port);
						// 	auto serial		= sp_get_port_usb_serial(*port);

						// 	std::cout << "\tManufacturer: " << (man ? man : "-") << std::endl;
						// 	std::cout << "\t     Product: " << (product ? product : "-") << std::endl;
						// 	std::cout << "\t      Serial: " << (serial ? serial : "-") << std::endl;
						// }
					}

					sp_free_port_list(list);
				}

				return result;
			}


			bool Connected()
			{
				return this->serial;
			}


			// Set the UI LED colour
			bool Ui(uint8_t address, uint8_t r, uint8_t g, uint8_t b) { return Write<2>(address, HPFC01::UiLed, { uint16_t((g << 8) + r), b }); }


			// Gets and sets the "time on" values of banks 1-4 for the given quick select group
			Result<4> Group(uint8_t address, uint8_t group)										{ return group < 0x10 ? Read<4>(address, HPFC01::TimeOnStart + (group << 2)) : Result<4>(); }
			bool Group(uint8_t address, uint8_t group, const std::array<uint16_t, 4> &values)	{ return group < 0x10 ? Write(address, HPFC01::TimeOnStart + (group << 2), values) : false; }

			// Gets and sets the "time on" values for continuous mode
			Result<4> Continuous(uint8_t address)										{ return Read<4>(address, HPFC01::TimeOnContinuous); }
			bool Continuous(uint8_t address, const std::array<uint16_t, 4> &values)		{ return Write(address, HPFC01::TimeOnContinuous, values); }


			bool EnableContinuous(uint8_t address, bool enable) { return SetBit(address, HPFC01::Config1, HPFC01::Configuration::Continuous, enable); }

			// Only applicable in continuous mode, changes the timing for camera triggering to reduce power consumption where appropriate
			bool EnablePreemptiveTiming(uint8_t address, bool enable) { return SetBit(address, HPFC01::Config1, HPFC01::Configuration::PreemptiveTiming, enable); }


			// Applicable when not in continuous mode. Switches the triggering polarity so that the flash is triggered on falling edge instead of rising.
			bool EnableFallingEdge(uint8_t address, bool enable) { return SetBit(address, HPFC01::Config1, HPFC01::Configuration::FallingEdge, enable); }


			// Expert mode for setting Config1 - will override all bits in the configuration with the given value
			bool SetConfiguration(uint8_t address, uint16_t value) { return Write<1>(address, HPFC01::Config1, { value }); }



			// First value is the driver temperature, second value is the LED board temperature. Both are in degrees Celsius.
			std::array<double, 2> Temperature(uint8_t address)
			{
				auto [success, data] = Read<2>(address, HPFC01::Temperature);

				if (success)
				{
					return {
						0.00390625 * (data[0] & 0x8000 ? -1 : 1)  * (data[0] & 0x7fff),
						0.00390625 * (data[1] & 0x8000 ? -1 : 1)  * (data[1] & 0x7fff)
					};
				}
				return { 0 };
			}


			std::string ID(uint8_t address)
			{
				auto [success, data] = Read<5>(address, HPFC01::ID);

				return success
					? emg::String::format("%04x%04x%04x%04x%04x", data[4], data[3], data[2], data[1], data[0])
					: "unknown";
			}


			// Sets the address of the connected device
			bool Address(uint8_t address, uint8_t value)		{ return Write<1>(address, HPFC01::Address, { value }); }
			Result<2> Version(uint8_t address)					{ return Read<2>(address, HPFC01::Hardware); }
			Result<1> CurrentLevel(uint8_t address)				{ return Read<1>(address, HPFC01::CurrentLevel); }
			bool CurrentLevel(uint8_t address, uint8_t value)	{ return Write<1>(address, HPFC01::CurrentLevel, { value }); }
			Result<1> VoltageLevel(uint8_t address)				{ return Read<1>(address, HPFC01::VoltageLevel); }
			bool VoltageLevel(uint8_t address, uint8_t value)	{ return Write<1>(address, HPFC01::VoltageLevel, { value }); }
			double VoltageIn(uint8_t address)
			{
				auto [success, data] = Read<1>(address, HPFC01::VoltageIn);
				return success ? data[0] * 0.001220703 / 0.227053 : 0;
			}
			double VoltageOut(uint8_t address)
			{
				auto [success, data] = Read<1>(address, HPFC01::VoltageOut);
				return success ? data[0] * 0.001220703 / 0.135447 : 0;
			}

			// Save the current registers to the internal EEROM so that they will persist after a power cycle.
			bool Save(uint8_t address) { return SetBit(address, HPFC01::Config0, HPFC01::Configuration::Save, true); }

			// Overwrite the current register values with those in the internal EEROM (as if it had been power cycled).
			bool Load(uint8_t address) { return SetBit(address, HPFC01::Config0, HPFC01::Configuration::Load, true); }


			// Enable watchdog timer and reset based on receiving communications, flash triggers or both.
			bool Watchdog(uint8_t address, bool comms, bool trigger)
			{
				auto [success, data] = this->Read<1>(address, HPFC01::Config0);

				if (success)
				{
					data[0] = comms		? (data[0] | HPFC01::Configuration::WatchdogComms)	: (data[0] & ~HPFC01::Configuration::WatchdogComms);
					data[0] = trigger	? (data[0] | HPFC01::Configuration::WatchdogTrigger) : (data[0] & ~HPFC01::Configuration::WatchdogTrigger);

					return this->Write<1>(address, HPFC01::Config0, data);
				}

				return false;
			}

			// Enable alternative trigger mode - polarity controls what level the data line must be to allow triggering
			bool AlternativeTrigger(uint8_t address, bool enable, bool polarity)
			{
				auto [success, data] = this->Read<1>(address, HPFC01::Config1);

				if (success)
				{
					data[0] = enable	? (data[0] | HPFC01::Configuration::AlternativeTrigger)	: (data[0] & ~HPFC01::Configuration::AlternativeTrigger);
					data[0] = polarity	? (data[0] | HPFC01::Configuration::AlternativePolarity) : (data[0] & ~HPFC01::Configuration::AlternativePolarity);

					return this->Write<1>(address, HPFC01::Config1, data);
				}
				return false;
			}


			// Read from an i2c device (such as the LED board) using the extension registers
			const Result<1> Extension(uint8_t address, uint16_t deviceAddress, uint16_t registerAddress)
			{
				if (Write<2>(address, HPFC01::ExtensionAddress, { deviceAddress, registerAddress }))
				{
					return Read<1>(address, HPFC01::ExtensionData);
				}

				return { false, {} };
			}


			// Write to an i2c device (such as the LED board) using the extension registers
			bool Extension(uint8_t address, uint16_t deviceAddress, uint16_t registerAddress, uint16_t value)
			{
				return Write<3>(address, HPFC01::ExtensionAddress, { deviceAddress, registerAddress, value });
			}


			const Result<2> Config(uint8_t address)
			{
				return Read<2>(address, HPFC01::Config0);
			}


			// Hardware version and ID of LED board
			std::string LedHardware(uint8_t address)
			{
				auto [success, data] = Read<2>(address, HPFC01::LedHardware);
				return success
					? emg::String::format("%04x%04x", data[1], data[0])
					: "--------";
			}

			std::string LedID(uint8_t address)
			{
				auto [success, data] = Read<4>(address, HPFC01::LedID);
				return success
					? emg::String::format("%04x%04x%04x%04x", data[3], data[2], data[1], data[0])
					: "----------------";
			}

			Status GetStatus()	{ return this->status; }
			void ResetStatus()	{ this->status = {}; }


		private:


			bool SetBit(uint8_t address, uint8_t registerAddress, uint8_t mask, bool value)
			{
				auto [success, data] = this->Read<1>(address, registerAddress);

				if (success)
				{
					data[0]	= value ? (data[0] | mask) : (data[0] & ~mask);
					return this->Write<1>(address, registerAddress, data);
				}

				return false;
			}


			template <size_t N> bool Write(uint8_t address, uint8_t registerAddress, const std::array<uint16_t, N> &values)
			{
				if (!this->serial && !this->Connect())
				{
				 	return false;
				}

				for (uint8_t i=0; i<N; i++)
				{
					if (!this->Write(address, registerAddress + i, values[i], true))
					{
						return false;
					}

					std::this_thread::sleep_for(10ms);
				}

				return true;
			}


			template <size_t N> const Result<N> Read(uint8_t address, uint8_t registerAddress)
			{
				if (!this->serial && !this->Connect())
				{
					return { false, {} };
				}

				std::array<uint16_t, N> result = { 0 };

				for (uint8_t i=0; i<N; i++)
				{
					if (!this->Write(address, registerAddress + i, 0x00, false))
					{
						return { false, {} };
					}

					if (!this->Read(result[i]))
					{
						return { false, {} };
					}
				}

				return { true, result };
			}


			bool Write(uint8_t address, uint8_t registerAddress, uint16_t value, bool set)
			{
				this->status.write++;
				uint8_t buffer[7]			= { address, 0, set ? uint8_t(registerAddress ^ 0x80) : registerAddress };
				*(uint16_t *)(buffer + 3)	= value;
				*(uint16_t *)(buffer + 5)	= CRC(buffer);

				if (!Check(sp_blocking_write(this->serial, buffer, 7, 50), 7))
				{
					this->status.errors++;
					return false;
				}

				return true;
			}


			bool Read(uint16_t &value)
			{
				this->status.read++;
				uint8_t buffer[7] = { 0 };

				if (!Check(sp_blocking_read(this->serial, buffer, 7, 50), 7))
				{
					this->status.errors++;
					return false;
				}

				value = *(uint16_t *)(buffer + 3);


				if (CRC(buffer) != *(uint16_t *)(buffer + 5))
				{
					this->status.crc++;
					return false;
				}

				return true;
			}


			bool Check(sp_return result, int expected)
			{
				if ((int)result == expected)
				{
					this->errors = 0;
					return true;
				}

				if (++this->errors > 4)
				{
					emg::Log::Error(
						"Multiple errors reading/writing to flash on '%s', attempting reconnection",
						this->connection.c_str()
					);

					this->Free();
				}

				return false;
			}


			uint16_t CRC(uint8_t buffer[5])
			{
				uint16_t result = 0xffff;

				for (int i=0; i<5; i++)
				{
					result ^= buffer[i];

					for (int j=8; j>0; j--)
					{
						if (result & 0x01)	result = (result >> 1) ^ 0xa001;
						else				result = result >> 1;
					}
				}

				return result;
			}


			bool Connect()
			{
				this->Free();

				if (!this->connection.empty())
				{
					#ifdef __linux__
						const auto canonical = fs::exists(this->connection)
							? fs::canonical(this->connection).string()
							: this->connection;
					#else
						const auto canonical = this->connection;
					#endif

					if (sp_get_port_by_name(canonical.c_str(), &this->serial) == SP_OK)
					{
						if (sp_open(this->serial, SP_MODE_READ_WRITE) == SP_OK)
						{
							sp_set_baudrate(this->serial, 9600);
							sp_set_bits(this->serial, 8);
							sp_set_parity(this->serial, SP_PARITY_NONE);
							sp_set_stopbits(this->serial, 1);
							sp_set_flowcontrol(this->serial, SP_FLOWCONTROL_NONE);

							sp_set_dtr(this->serial, SP_DTR_OFF);
							sp_set_rts(this->serial, SP_RTS_OFF);

							sp_flush(this->serial, SP_BUF_BOTH);

							emg::Log::Info("Flash control connected on '%s'", this->connection);
						}
						else
						{
							sp_free_port(this->serial);
							this->serial = nullptr;
						}
					}
				}

				return this->serial;
			}


			void Free(bool close = true)
			{
				if (this->serial)
				{
					emg::Log::Info("Flash control disconnecting on '%s'", this->connection);

					if (close)
					{
						sp_close(this->serial);
					}

					sp_free_port(this->serial);
					this->serial = nullptr;
				}
			}

			sp_port *serial = nullptr;
			uint8_t empty	= 0;
			int errors		= 0;



			std::string connection;
			emg::Timer last;
			Status status;
	};

}

