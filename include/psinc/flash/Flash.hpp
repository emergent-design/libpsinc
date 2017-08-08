#pragma once

#include <string>
#include <emergent/String.hpp>
#include <emergent/Timer.hpp>
#include <emergent/logger/Logger.hpp>
// #include <modbus/modbus.h>
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


				// The slow flag forces the communications to transmit one byte at a time. This should
				// only be used when communicating with a flash via a camera. If there is a direct serial
				// connection to the flash then this should be left disabled.
				bool Initialise(const std::string &connection, uint8_t address, bool slow = false)
				{
					this->connection	= connection;
					this->address		= address;
					this->slow			= slow;

					return this->Connect();
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

						if (this->slow)
						{
							std::this_thread::sleep_for(2ms);
						}
					}

					return true;
				}


				template <size_t N> const std::array<uint16_t, N> Read(uint8_t address)
				{
					if (!this->serial && !this->Connect())
					{
						return {};
					}

					std::array<uint16_t, N> result = { 0 };

					for (uint8_t i=0; i<N; i++)
					{
						if (!this->Write(address + i, 0x00, false))
						{
							return {};
						}

						if (!this->Read(result[i]))
						{
							return {};
						}

						if (this->slow)
						{
							std::this_thread::sleep_for(2ms);
						}
					}

					return result;
				}


				bool Write(uint8_t address, uint16_t value, bool set)
				{
					uint8_t buffer[7]			= { this->address, 0, set ? uint8_t(address ^ 0x80) : address };
					*(uint16_t *)(buffer + 3)	= value;
					*(uint16_t *)(buffer + 5)	= CRC(buffer);

					if (this->slow)
					{
						for (int i=0; i<7; i++)
						{
							if (!Check(sp_blocking_write(this->serial, buffer + i, 1, 10), 1, "write"))
							{
								return false;
							}
							std::this_thread::sleep_for(1ms);
						}
					}
					else
					{
						if (!Check(sp_blocking_write(this->serial, buffer, 7, 50), 7, "write"))
						{
							return false;
						}
					}

					return true;
				}


				bool Read(uint16_t &value)
				{
					uint8_t buffer[7] = { 0 };

					if (this->slow)
					{
						for (int i=0; i<7; i++)
						{
							if (!Check(sp_blocking_read(this->serial, buffer + i, 1, 10), 1, "read"))
							{
								return false;
							}
							std::this_thread::sleep_for(1ms);
						}
					}
					else
					{
						if (!Check(sp_blocking_read(this->serial, buffer, 7, 50), 7, "read"))
						{
							return false;
						}
					}

					if (CRC(buffer) != *(uint16_t *)(buffer + 5))
					{
						emg::Log::Error("Flash control read CRC error");
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
						if (sp_open(this->serial, (sp_mode)3) == SP_OK)
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
				bool slow		= false;

				std::string connection;
				emg::Timer last;
		};

	}
}
