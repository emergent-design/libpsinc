#pragma once

#include <emergent/Emergent.hpp>


namespace psinc
{
	namespace flash
	{
		struct HPFC01
		{
			static const uint8_t Address 			= 0x00;		// Sets the device modbus address
			static const uint8_t CurrentLevel		= 0x01;		// Sets the current level through each bank of LEDS 0x00 - 0x40
			static const uint8_t VoltageLevel		= 0x02;		// Sets the voltage level for the driver circuit 0x00 - 0x80

			// Each of the following is the address for bank 1, the remain banks (2-4) are consecutively addressed
			static const uint8_t TimeOnStart		= 0x03;		// Sets 16-bit ratio of on time for quick select 0 through to f
			static const uint8_t TimeOnContinuous	= 0x43;	 	// Sets 16-bit ratio of on time for continuous operation

			static const uint8_t Config0			= 0x47;		// Configuration register 0
			static const uint8_t Config1			= 0x48;		// Configuration register 1

			static const uint8_t ExtensionAddress	= 0x49;		// Extension slave address for i2c devices on the LED boards
			static const uint8_t ExtensionRegister	= 0x4a;		// Register address to use when reading/writing extension data
			static const uint8_t ExtensionData		= 0x4b;		// Data to read/write to the slave i2c device

			static const uint8_t UiLed				= 0x4c;		// Red/green feedback LED control, blue is the next address
			static const uint8_t Status				= 0x4e;
			static const uint8_t VoltageIn			= 0x4f;
			static const uint8_t VoltageOut			= 0x50;

			static const uint8_t ID					= 0x51;		// Device ID low byte - subsequent 4 addresses are remaining bytes of ID
			static const uint8_t Hardware			= 0x56;		// Hardware version
			static const uint8_t Firmware			= 0x57;		// Firmware version

			static const uint8_t Temperature		= 0x58;		// Driver board temperature sensor
			static const uint8_t LedTemperature		= 0x59;		// LED board temperature sensor
			static const uint8_t LedHardware		= 0x5a;		// LED board hardware version
			static const uint8_t LedExtHardware		= 0x5b;	 	// LED board hardware extension version
			static const uint8_t LedID				= 0x5c;		// LED board ID low byte - subsequent 3 addresses are remaining bytes of ID


			// Bit masks for configuration register Config1
			struct Configuration
			{
				static const uint16_t Continuous		= 0b00000001;
				static const uint16_t PreemptiveTiming	= 0b00000010;
				static const uint16_t FallingEdge		= 0b00000100;
			};
		};
	}
}



