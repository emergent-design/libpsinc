#pragma once

#include <string>
#include <regex>
#include <emergent/String.hpp>
#include <emergent/Timer.hpp>
#include <emergent/logger/Logger.hpp>
#include <psinc/flash/HPFC.hpp>
#include <libserialport.h>


namespace psinc
{
	namespace flash
	{
		using namespace std::chrono;

		// A helper class for communicating with PSI flash driver units over RS485. A binary that uses this header
		// must link with libserialport.
		class Flash
		{
			public:

				~Flash()
				{
					this->Free();
				}


				// Set the serial port connection string and device address, then initiate a connection
				bool Initialise(const std::string &connection, uint8_t address)
				{
					this->connection	= connection;
					this->address		= address;

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
								auto id = emg::String::trim(sp_get_port_usb_serial(*port), ' ');

								if (std::regex_match(id, std::regex(serial)))
								{
									result = sp_get_port_name(*port);
									break;
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
				bool Ui(uint8_t r, uint8_t g, uint8_t b) { return Write<2>(HPFC01::UiLed, { uint16_t((g << 8) + r), b }); }


				// Gets and sets the "time on" values of banks 1-4 for the given quick select group
				std::array<uint16_t, 4> Group(uint8_t group)						{ return group < 0x10 ? Read<4>(HPFC01::TimeOnStart + (group << 2)) : std::array<uint16_t, 4>(); }
				bool Group(uint8_t group, const std::array<uint16_t, 4> &values)	{ return group < 0x10 ? Write(HPFC01::TimeOnStart + (group << 2), values) : false; }

				// Gets and sets the "time on" values for continuous mode
				std::array<uint16_t, 4> Continuous()								{ return Read<4>(HPFC01::TimeOnContinuous); }
				bool Continuous(const std::array<uint16_t, 4> &values)				{ return Write(HPFC01::TimeOnContinuous, values); }
				bool EnableContinuous(bool enable)									{ return Write<1>(HPFC01::Config1, { uint16_t(enable ? 0x01 : 0x00) }); }


				// First value is the driver temperature, second value is the LED board temperature. Both are in degrees Celsius.
				std::array<double, 2> Temperature()
				{
					auto values = Read<2>(HPFC01::Temperature);

					return {
						0.00390625 * (values[0] & 0x8000 ? -1 : 1)  * (values[0] & 0x7fff),
						0.00390625 * (values[1] & 0x8000 ? -1 : 1)  * (values[1] & 0x7fff)
					};
				}


				std::string ID()
				{
					auto values = Read<5>(HPFC01::ID);

					return emg::String::format("%04x%04x%04x%04x%04x", values[4], values[3], values[2], values[1], values[0]);
				}


				// Sets the address of the connected device
				bool Address(uint8_t value)			{ return Write<1>(HPFC01::Address, { value }); }
				std::array<uint16_t, 2> Version()	{ return Read<2>(HPFC01::Hardware); }
				uint8_t CurrentLevel()				{ return Read<1>(HPFC01::CurrentLevel)[0]; }
				bool CurrentLevel(uint8_t value)	{ return Write<1>(HPFC01::CurrentLevel, { value }); }
				uint8_t VoltageLevel()				{ return Read<1>(HPFC01::VoltageLevel)[0]; }
				bool VoltageLevel(uint8_t value)	{ return Write<1>(HPFC01::VoltageLevel, { value }); }
				double VoltageIn()					{ return Read<1>(HPFC01::VoltageIn)[0] * 0.001220703 / 0.227053; }
				double VoltageOut()					{ return Read<1>(HPFC01::VoltageOut)[0] * 0.001220703 / 0.135447; }

				// Save the current registers to the internal EEROM so that they will persist after a power cycle.
				bool Save() { return Write<1>(HPFC01::Config0, { 0x02 }); }

				// Overwrite the current register values with those in the internal EEROM (as if it had been power cycled).
				bool Load() { return Write<1>(HPFC01::Config0, { 0x01 }); }


			private:

				template <size_t N> bool Write(uint8_t address, const std::array<uint16_t, N> &values)
				{
					if (!this->serial && !this->Connect())
					{
					 	return false;
					}

					for (uint8_t i=0; i<N; i++)
					{
						if (!this->Write((uint8_t)address + i, values[i], true))
						{
							return false;
						}

						std::this_thread::sleep_for(10ms);
					}

					return true;
				}


				template <size_t N> const std::array<uint16_t, N> Read(uint8_t address)
				{
					if (!this->serial && !this->Connect())
					{
						return { 0 };
					}

					std::array<uint16_t, N> result = { 0 };

					for (uint8_t i=0; i<N; i++)
					{
						if (!this->Write(address + i, 0x00, false))
						{
							return { 0 };
						}

						if (!this->Read(result[i]))
						{
							return { 0 };
						}
					}

					return result;
				}


				bool Write(uint8_t address, uint16_t value, bool set)
				{
					uint8_t buffer[7]			= { this->address, 0, set ? uint8_t(address ^ 0x80) : address };
					*(uint16_t *)(buffer + 3)	= value;
					*(uint16_t *)(buffer + 5)	= CRC(buffer);

					if (!Check(sp_blocking_write(this->serial, buffer, 7, 50), 7, "write"))
					{
						return false;
					}

					return true;
				}


				bool Read(uint16_t &value)
				{
					uint8_t buffer[7] = { 0 };

					if (!Check(sp_blocking_read(this->serial, buffer, 7, 50), 7, "read"))
					{
						return false;
					}

					value = *(uint16_t *)(buffer + 3);
					return true;
				}


				bool Check(sp_return result, int expected, const char *which)
				{
					if ((int)result == expected) return true;

					emg::Log::Error("Flash control %s error: %s", which, sp_last_error_message());

					this->Free();

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

					if (sp_get_port_by_name(this->connection.c_str(), &this->serial) == SP_OK)
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
				uint8_t address	= 0;
				uint8_t empty	= 0;

				std::string connection;
				emg::Timer last;
		};

	}
}

