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
		};

		// enum class HPFC01 : byte
		// {
		// 	Address 			= 0x00,	// Sets the device modbus address
		// 	CurrentLevel		= 0x01,	// Sets the current level through each bank of LEDS 0x00 - 0x40
		// 	VoltageLevel		= 0x02,	// Sets the voltage level for the driver circuit 0x00 - 0x80

		// 	// Each of the following is the address for bank 1, the remain banks (2-4) are consecutively addressed
		// 	TimeOn0				= 0x03, // Sets 16-bit ratio of on time for quick select 0
		// 	TimeOn1				= 0x07,	// Sets 16-bit ratio of on time for quick select 1
		// 	TimeOn2				= 0x0b,	// Sets 16-bit ratio of on time for quick select 2
		// 	TimeOn3				= 0x0f,	// Sets 16-bit ratio of on time for quick select 3
		// 	TimeOn4				= 0x13,	// Sets 16-bit ratio of on time for quick select 4
		// 	TimeOn5				= 0x17,	// Sets 16-bit ratio of on time for quick select 5
		// 	TimeOn6				= 0x1b,	// Sets 16-bit ratio of on time for quick select 6
		// 	TimeOn7				= 0x1f,	// Sets 16-bit ratio of on time for quick select 7
		// 	TimeOn8				= 0x23,	// Sets 16-bit ratio of on time for quick select 8
		// 	TimeOn9				= 0x27,	// Sets 16-bit ratio of on time for quick select 9
		// 	TimeOnA				= 0x2b,	// Sets 16-bit ratio of on time for quick select A
		// 	TimeOnB				= 0x2f,	// Sets 16-bit ratio of on time for quick select B
		// 	TimeOnC				= 0x33,	// Sets 16-bit ratio of on time for quick select C
		// 	TimeOnD				= 0x37,	// Sets 16-bit ratio of on time for quick select D
		// 	TimeOnE				= 0x3b,	// Sets 16-bit ratio of on time for quick select E
		// 	TimeOnF				= 0x3f,	// Sets 16-bit ratio of on time for quick select F
		// 	TimeOnContinuous	= 0x43, // Sets 16-bit ratio of on time for continuous operation

		// 	Config0				= 0x47,	// Configuration register 0
		// 	Config1				= 0x48,	// Configuration register 1

		// 	ExtensionAddress	= 0x49,	// Extension slave address for i2c devices on the LED boards
		// 	ExtensionRegister	= 0x4a,	// Register address to use when reading/writing extension data
		// 	ExtensionData		= 0x4b,	// Data to read/write to the slave i2c device

		// 	UiLed1				= 0x4c,	// Red/green feedback LED control
		// 	UiLed2				= 0x4d,	// Blue feedback LED control
		// 	Status				= 0x4e,
		// 	VoltageIn			= 0x4f,
		// 	VoltageOut			= 0x50,

		// 	ID					= 0x51,	// Device ID low byte - subsequent 4 addresses are remaining bytes of ID
		// 	Hardware			= 0x56,	// Hardware version
		// 	Firmware			= 0x57,	// Firmware version

		// 	Temperature			= 0x58,	// Driver board temperature sensor
		// 	LedTemperature		= 0x59,	// LED board temperature sensor
		// 	LedHardware			= 0x5a,	// LED board hardware version
		// 	LedExtHardware		= 0x5b, // LED board hardware extension version
		// 	LedID				= 0x5c,	// LED board ID low byte - subsequent 3 addresses are remaining bytes of ID
		// };


// #define HPFC01_WRITE  ^0x80
// #define HPFC01_SAVE 0x02
// #define HPFC01_VIN_CALC(x) ((x*0.001220703)/0.227053)
// #define HPFC01_VOUT_CALC(x) ((x*0.001220703)/0.135447)
// #define HPFC01_T_CALC(x) ((x & 0x7fff)*0.00390625);

// #define HPFC01_LED_HARDWARE_ADDRESS_W 0x8051
// #define HPFC01_LED_HARDWARE_ADDRESS 0x0051
// #define HPFC01_LED_ID_ADD_L 0x0010
// #define HPFC01_LED_ID_ADD_ML 0x0012
// #define HPFC01_LED_ID_ADD_MH 0x0014
// #define HPFC01_LED_ID_ADD_H 0x0016
// #define HPFC01_LED_TYPE_ADD_L 0x000C
// #define HPFC01_LED_TYPE_ADD_H 0x000E
// #endif // HPFC01_H


	}
}
