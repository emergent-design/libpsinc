#pragma once
#include <algorithm>
#include <cstdint>


namespace psinc
{
	struct Properties
	{
		// Values between 0.0 and 1.0 representing the minimum and maximum sensible values for the underlying device
		double exposure = 0.0;
		double gain		= 1.0;

		// Colour balance - gain values for each colour channel (greenR and greenB are assumed to be the same)
		double red		= 1.0;
		double green	= 1.0;
		double blue		= 1.0;

		// 0 = off, non-zero = on or specific flash setting depending on the camera
		uint8_t flash = 0;


		Properties &Flash(const uint8_t value)		{ this->flash		= value;						return *this; }
		Properties &Exposure(const double value)	{ this->exposure	= std::clamp(value, 0.0, 1.0);	return *this; }
		Properties &Gain(const double value)		{ this->gain		= std::clamp(value, 0.0, 1.0);	return *this; }
		Properties &Red(const double value)			{ this->red			= std::clamp(value, 0.0, 7.97);	return *this; }
		Properties &Green(const double value)		{ this->green		= std::clamp(value, 0.0, 7.97);	return *this; }
		Properties &Blue(const double value)		{ this->blue		= std::clamp(value, 0.0, 7.97);	return *this; }
	};
}

