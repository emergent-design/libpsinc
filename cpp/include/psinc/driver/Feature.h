#pragma once

#include <emergent/Emergent.h>
#include <emergent/struct/Bounds.h>
#include <psinc/driver/Register.h>
#include <memory>
#include <map>
#include <set>


namespace psinc
{
	/// Represents a specific chip feature and its value
	class Feature
	{
		public:

			/// Default constructor
			Feature();


			virtual ~Feature() {}


			/// Constructor.
			/// @param[in] configuration XML containing the feature configuration
			/// @param[in] parent Pointer to the parent register (if there is one)
			Feature(pugi::xml_node configuration, Register *parent);


			/// Set the value of this feature. Will return false if the supplied value is out of range
			virtual bool Set(int value);


			/// Get the current value of this feature
			virtual int Get();


			/// Refresh this feature by requesting an update of the underlying register
			/// value from the camera
			virtual bool Refresh();


			/// Get the size of this feature (in bits)
			int Size();


			/// Reset this feature to the default value (as defined in the chip XML)
			int Reset();


			/// Return the minimum permitted value for this feature
			int Minimum();


			/// Return the maximum permitted value for this feature
			int Maximum();


		private:

			/// Determine the invalid values from the given list
			void Invalidate(std::string values);


			/// The bitmask for this feature, since multiple features
			/// can exist within a single register.
			int mask;

			/// The minimum permitted value for this feature
			int minimum;

			/// The maximum permitted value for this feature
			int maximum;

			/// Default value for this feature
			int defaultValue;

			/// Range(s) of valid values
			std::set<int> invalid;

			/// Number of bits this feature uses
			int bits;

			/// Indicates whether or not this feature is a flag
			bool flag;

			/// Offset of this feature within the register
			int offset;

			/// True if this feature cannot be written to
			bool readonly;

			/// Parent register for this feature
			Register *parent;
	};
}
