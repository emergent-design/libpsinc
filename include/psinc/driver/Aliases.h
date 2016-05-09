#pragma once

#include <psinc/driver/Feature.h>

namespace psinc
{

	// Aliases for common features within a single context.
	class Aliases
	{
		public:

			Feature *width				= &unused;
			Feature *height				= &unused;
			Feature *gain				= &unused;
			Feature *exposure			= &unused;
			Feature *autoGain			= &unused;
			Feature *autoExposure		= &unused;

			Feature *columnStart		= &unused;
			Feature *columnEnd		 	= &unused;
			Feature *rowStart			= &unused;
			Feature *rowEnd				= &unused;

			Feature *companding			= &unused;
			Feature *noiseCorrection	= &unused;
			Feature *adcReference		= &unused;

			Feature *context			= &unused;


			Feature *operator [](std::string key)
			{
				return this->lookup.count(key) ? *this->lookup[key] : &this->unused;
			}


			bool Contains(std::string key)
			{
				return this->lookup.count(key);
			}


			void Set(std::string key, Feature &feature)
			{
				if (this->lookup.count(key))
				{
					*this->lookup[key] = &feature;
				}
			}


		private:

			Feature unused;

			std::map<std::string, Feature **> lookup = {
				{ "Width",				&width },
				{ "Height",				&height },
				{ "Gain",				&gain },
				{ "Exposure",			&exposure },
				{ "AutoGain",			&autoGain },
				{ "AutoExposure",		&autoExposure },
				{ "ColumnStart",		&columnStart },
				{ "ColumnEnd",			&columnEnd },
				{ "RowStart",			&rowStart },
				{ "RowEnd",				&rowEnd },
				{ "Companding",			&companding },
				{ "NoiseCorrection",	&noiseCorrection },
				{ "ADCReference",		&adcReference },
				{ "Context",			&context }
			};
	};
}


