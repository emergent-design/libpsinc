#pragma once

#include <emergent/Emergent.h>
#include <emergent/struct/Bounds.h>
#include <psinc/driver/Register.h>
#include <memory>
#include <map>
#include <set>


namespace psinc
{
	/// Represents a specific device feature and its value
	class Feature
	{
		public:
			/// Default constructor
			Feature();

			virtual ~Feature() {}

			/// Constructor.
			/// @param[in] configuration XML containing the feature configuration
			/// @param[in] parent Pointer to the parent register (if there is one)
			/// @param[in] offset Bit offset for the feature within the register
			Feature(pugi::xml_node configuration, Register *parent);

			/// Set the value of this feature. Will return false if the supplied value is out of range
			virtual bool Set(int value);

			/// Get the current value of this feature
			virtual int Get();
			
			virtual bool Refresh();

			/// Get the size of this feature (in bits)
			int Size();
			
			int Reset();

			int Minimum();
			int Maximum();

		
		private:
		
			void Invalidate(std::string values);
			
			int mask;
			int minimum;
			int maximum;

			/// Default value for this feature
			int defaultValue;

			/// Range(s) of valid values
			std::set<int> invalid;

			/// Number of bits this feature uses
			int bits;
			
			bool flag;

			/// Offset of this feature within the register
			int offset;

			/// True if this feature cannot be written to
			bool readonly;

			/// Parent register for this feature
			Register *parent;
	};

	/// Definition for the map of features owned by the camera
	//typedef std::map<std::string, std::unique_ptr<Feature>> FeatureMap;
	//typedef std::map<byte, std::map<std::string, Feature *>> AliasMap;
}
