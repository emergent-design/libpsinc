#include "psinc/driver/Feature.h"

#include <emergent/logger/Logger.hpp>

using namespace std;
using namespace pugi;
using namespace emergent;


namespace psinc
{
	Feature::Feature()
	{
	}


	Feature::Feature(xml_node configuration, Register *parent)
	{
		this->parent		= parent;
		this->defaultValue	= configuration.attribute("default").as_int();
		this->offset		= configuration.attribute("offset").as_int();
		this->bits			= configuration.attribute("bits").as_int();
		this->readonly		= configuration.attribute("readonly").as_bool();
		this->minimum		= configuration.attribute("min").as_int();
		this->maximum		= configuration.attribute("max").as_int(1);
		this->flag			= this->bits == 1;
		this->mask			= 0;

		for (int i=0; i<this->bits; i++) this->mask |= 1 << (this->offset + i);

		this->Invalidate(configuration.attribute("invalid").as_string());
	}


	void Feature::Invalidate(string values)
	{
		if (values.size())
		{
			char *check;
			auto ranges = String::explode(values, ",");

			for (string &range : ranges)
			{
				if (range.size())
				{
					auto limits	= String::explode(range, "-");
					int start	= strtol(limits[0].c_str(), &check, 0);

					if (check > limits[0].c_str())
					{
						if (limits.size() > 1)
						{
							int end	= strtol(limits[1].c_str(), &check, 0);

							if (check < limits[1].c_str())
							{
								for (int i=start; i<=end; i++) this->invalid.insert(i);
							}
						}
						else this->invalid.insert(start);
					}
				}
			}
		}
	}


	bool Feature::Valid(int value)
	{
		return value >= this->minimum && value <= this->maximum && !this->invalid.count(value);
	}


	bool Feature::Set(int value)
	{
		if (this->parent && !this->readonly && this->Valid(value))
		{
			return this->flag
				? this->parent->SetBit(this->offset, value)
				: this->parent->Set(this->offset, this->mask, value);
		}

		return false;
	}


	bool Feature::SetLow()
	{
		return this->Set(this->minimum);
	}


	bool Feature::SetHigh()
	{
		return this->Set(this->maximum);
	}


	bool Feature::SetDefault()
	{
		return this->Set(this->defaultValue);
	}


	bool Feature::Set(bool high)
	{
		return high ? this->SetHigh() : this->SetLow();
	}


	int Feature::Get()
	{
		return this->parent ? (this->parent->Get() & this->mask) >> this->offset : 0;
	}


	int Feature::Size()
	{
		return bits;
	}


	bool Feature::Refresh()
	{
		return this->parent->Refresh();
	}


	int Feature::Reset()
	{
		return this->Set(this->defaultValue) ? this->defaultValue : this->Get();
	}


	int Feature::Minimum()
	{
		return this->minimum;
	}


	int Feature::Maximum()
	{
		return this->maximum;
	}


	bool Feature::ReadOnly()
	{
		return this->readonly;
	}
}
