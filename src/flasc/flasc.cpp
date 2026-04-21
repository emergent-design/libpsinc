#include "flasc/flasc.hpp"
#include "flasc/bootstrap.hpp"

#include <emergent/Clap.hpp>
#include <entity/entity.hpp>
#include <entity/bson.hpp>
#include <entity/json.hpp>
#include <psinc/Camera.h>
#include <psinc/Psinc.h>
#include <emergent/FS.hpp>

#include <iostream>
#include <errno.h>

#ifndef FLASC_VERSION
	#define FLASC_VERSION "1.0.0"
#endif


// ---------------------------------- Structures ---------------------------------- //
struct Storage
{
	enum class Type
	{
		Lens		= 0,
		Geometric	= 1
	};

	static emg::fs::path Path(int type)
	{
		switch (type)
		{
			case 0:		return emg::fs::path("cameras") / "lens" / "psinc";
			case 1:		return emg::fs::path("cameras") / "geometric" / "psinc";
			default:	return "";
		}
	}
};


struct Params
{
	int timeout = 500;
	string serial;
	string data;

	Params() {}

	Params(int timeout, const string &id, const string &serial, const string &data) : timeout(timeout), data(data)
	{
		// Construct the serial lookup string based on the specified ID and serial number.
		// For no ID or serial it is simply a wildcard ".*"
		// For ID but no serial it uses a wildcard serial ".*:{id}"
		// For serial but no ID is uses a wildcard ID "{serial}.*"
		// For serial and ID it uses a fully qualified lookup "{serial}:{id}"
		if (id.empty())				this->serial = serial + ".*";
		else if (serial.empty()) 	this->serial = ".*:" + id;
		else						this->serial = serial + ":" + id;
	}
};


struct Command
{
	std::function<void(Flasc &, const Params &)> operation;
	string description;
};



// ---------------------------------- Helper functions ---------------------------------- //


void print_result(const std::map<string, string> &result)
{
	for (auto &[k,v] : result)
	{
		std::cout << "  " << std::setw(12) << k << ": " << v << std::endl;
	}
}


// Convert a series of bytes to a hex string
std::string serialhex(const std::vector<byte> &serial)
{
	std::ostringstream result;
	result << std::hex << std::setfill('0');
	for (auto b : serial)
	{
		result << std::setw(2) << (int)b;
	}
	return result.str();
}


// Print the raw data as hex but highlight a particular position
std::string data_hex(const string &data, size_t position)
{
	auto d = (uint8_t *)data.data();
	std::ostringstream raw;
	raw << std::hex << std::setfill('0');

	for (size_t i=0; i<data.size(); i++)
	{
		if (i == position) raw << "\033[0;31m";
		raw << std::setw(2) << (int)*d++ << ' ';
	}
	raw << "\033[0m";
	return raw.str();
}


std::string read_device(Instrument &instrument, int channel)
{
	std::vector<byte> buffer(8192);

	std::cout << "  Reading from camera" << std::endl;

	instrument.devices["Storage0"].SetChannel(channel);
	instrument.devices["Storage0"].Read(buffer);

	if (buffer.size() == 8192 && std::all_of(buffer.begin(), buffer.end(), [](auto b) { return b == 0; }))
	{
		std::cout << "  Camera storage is uninitialised" << std::endl;
		return {};
	}

	return { buffer.begin(), buffer.end() };
}



// ---------------------------------- Commands ---------------------------------- //

void list(Flasc &, const Params &)
{
	for (auto &[k, v] : Instrument::List(Instrument::Type::Camera))
	{
		std::cout << "  " << v.address << " " << v.version << " - " << k << " - " << v.description << '\n';
	}
}


void version(Flasc &flasc, const Params &params)
{
	print_result(
		flasc.Version(params.serial)
	);
}


void flash(Flasc &flasc, const Params &params)
{
	flasc.Flash(params.serial, params.data);
}


void ramflash(Flasc &flasc, const Params &params)
{
	flasc.RamFlash(params.data, false);
}


void kick(Flasc &flasc, const Params &params)
{
	flasc.Kick(params.serial);
}


void serial(Flasc &flasc, const Params &)
{
	flasc.SetSerial();
}


void descriptor(Flasc &flasc, const Params &params)
{
	flasc.EnableSerial(params.serial);
}


void id(Flasc &flasc, const Params &params)
{
	flasc.SetId(params.serial, params.data);
}


void lighting(Flasc &flasc, const Params &params)
{
	flasc.SetLighting(params.serial, std::stoi(params.data));
}


void toggle(Flasc &flasc, const Params &params)
{
	flasc.Toggle(params.serial);
}


void watchdog(Flasc &flasc, const Params &params)
{
	flasc.SetWatchdog(params.serial, stoi(params.data));
}


void set_defaults(Flasc &flasc, const Params &params)
{
	string in;
	auto values = flasc.LoadDefaults(params.data);

	if (values.size())
	{
		std::cout << "  Will attempt to set the following register values:" << std::endl << std::hex << std::setfill('0');

		for (auto &v : values)
		{
			std::cout << "    Register 0x" << std::setw(2) << v.first << " to 0x" << std::setw(4) << v.second << std::endl;
		}

		std::cout << "  Continue [y/N]: " << std::flush;
		std::getline(std::cin, in);

		if (in == "y" || in == "Y") flasc.SetDefaults(params.serial, values);
	}
	else
	{
		std::cout << "  No values to set, reset the defaults on the camera [y/N]: " << std::flush;
		getline(std::cin, in);

		if (in == "y" || in == "Y") flasc.ResetDefaults(params.serial);
	}
}


void backup_storage(Flasc &flasc, const Params &params)
{
	const int channel = std::stoi(params.data);
	Instrument instrument;

	if (flasc.ConnectWait(instrument, Instrument::Type::Camera, params.serial, 100, 50))
	{
		auto buffer = read_device(instrument, channel);

		if (!buffer.empty())
		{
			auto data = ent::decode<ent::bson>(buffer);

			std::cout << "  Retrieved the following data " << std::endl;
			std::cout << ent::encode<ent::prettyjson>(data) << std::endl;

			string name = serialhex(instrument.devices["Serial"].Read());
			auto path	= emg::fs::path("backup") / Storage::Path(channel) / (name + ".bson");

			std::cout << "  Saving raw data to file " << path << std::endl;
			String::save(path.string(), buffer);
		}
		else
		{
			std::cout << "  Failed to read from channel " << channel << std::endl;
		}
	}
}


void read_storage(Flasc &flasc, const Params &params)
{
	const int channel = std::stoi(params.data);
	Instrument instrument;

	if (flasc.ConnectWait(instrument, Instrument::Type::Camera, params.serial, 100, 50))
	{
		auto buffer = read_device(instrument, channel);

		if (!buffer.empty())
		{
			// std::cout << buffer << std::endl;
			auto data = ent::decode<ent::bson>(buffer);

			std::cout << "  Retrieved the following data " << std::endl;
			std::cout << ent::encode<ent::prettyjson>(data) << std::endl;
		}
		else
		{
			std::cout << "  Failed to read from channel " << channel << std::endl;
		}
	}
}


void check_storage(Flasc &flasc, const Params &params)
{
	const int channel = std::stoi(params.data);

	std::vector<byte> buffer(8192);
	Instrument instrument;

	if (flasc.ConnectWait(instrument, Instrument::Type::Camera, params.serial, 100, 50))
	{
		string name	= serialhex(instrument.devices["Serial"].Read());
		auto path	= Storage::Path(channel) / (name + ".bson");

		std::cout << "  Looking for raw data file " << path << std::endl;

		if (emg::fs::exists(path))
		{
			auto source = String::load(path.string());
			auto buffer	= read_device(instrument, channel);

			if (buffer.size() == source.size())
			{
				auto result = std::mismatch(buffer.begin(), buffer.end(), source.begin());

				if (result.first == buffer.end())
				{
					std::cout << "  SUCCESS - camera data and file data match" << std::endl;
				}
				else
				{
					int position = std::distance(buffer.begin(), result.first);

					std::cout	<< "  Camera\n" << data_hex(buffer, position) << "\n\n"
								<< "  Disk\n" << data_hex(source, position) << "\n\n"
								<< "  FAILED - camera data and file data do not match at byte "
								<< std::distance(buffer.begin(), result.first)
								<< std::endl;
				}
			}
			else
			{
				std::cout << "  FAILED - camera data and file data differ in size " << buffer.size() << " -> " << source.size() << std::endl;
			}
		}
		else
		{
			std::cout << "  File does not exist, exiting" << std::endl;
		}
	}
}


void restore_storage(Flasc &flasc, const Params &params)
{
	const int channel = std::stoi(params.data);
	Instrument instrument;

	if (flasc.ConnectWait(instrument, Instrument::Type::Camera, params.serial, 100, 50))
	{
		string name = serialhex(instrument.devices["Serial"].Read());
		auto path	= Storage::Path(channel) / (name + ".bson");

		std::cout << "  Looking for raw data file " << path << std::endl;
		string data = String::load(path.string());

		if (!data.empty())
		{
			string in;
			auto decoded = ent::decode<ent::bson>(data);

			std::cout << "  Found the following data " << std::endl;
			std::cout << ent::encode<ent::prettyjson>(decoded) << std::endl;

			std::cout << "  Do you wish to proceed with the restore [y/N]: " << std::flush;
			getline(std::cin, in);

			if (in == "y" || in == "Y")
			{
				instrument.devices["Storage0"].SetChannel(channel);

				if (instrument.devices["Storage0"].Write(data))
				{
					std::cout << "  Completed successfully" << std::endl;
				}
				else
				{
					std::cout << "  Error writing to storage" << std::endl;
				}
			}
		}
		else
		{
			std::cout << "  File is empty or does not exist, exiting" << std::endl;
		}
	}
}


void reset(Flasc &flasc, const Params &params)
{
	Instrument instrument;

	if (flasc.ConnectWait(instrument, Instrument::Type::Camera, params.serial, 100, 50))
	{
		std::cout << "  Performing reset on device" << std::endl;
		instrument.Reset();
	}
}


void dump(Flasc &flasc, const Params &params)
{
	Camera camera;

	camera.Initialise(params.serial);

	if (flasc.ConnectWait(camera, Instrument::Type::Camera, params.serial, 100, 50))
	{
		for (auto &[k,f] : camera.features)
		{
			std::cout
				<< std::setw(28) << k << ": "
				<< std::setw(6) << std::dec << f.Get()
				<< " | 0x" << std::hex << f.Get() << '\n';
		}
	}
}


void bootstrap(Flasc &flasc, const Params &params)
{
	const string data = emg::String::load(params.data);

	if (data.empty())
	{
		std::cout << "  Bootstrap file is empty or does not exist, aborting\n";
		return;
	}

	auto configuration = ent::decode<ent::json, Bootstrap::Configuration>(data);

	Bootstrap::Go(flasc, configuration);
}


void command(const string &cmd, const Params &params)
{
	static const std::map<string, Command> commands = {
		{ "list",		{ &list,			"list connected devices" }},
		{ "version",	{ &version,			"display version information for a specific device" }},
		{ "flash",		{ &flash,			"flash firmware <data> file to device" }},
		{ "ramflash",	{ &ramflash,		"flash application directly to RAM - device must be in Cypress bootloader" }},
		{ "kick",		{ &kick,			"kick legacy device out of flash mode" }},
		{ "defaults",	{ &set_defaults,	"set device defaults from <data> file" }},
		{ "serial",		{ &serial,			"initialise a device with a serial number" }},
		{ "descriptor",	{ &descriptor,		"enable serial number in USB descriptor" }},
		{ "id",			{ &id,				"set the ID of the device to <data>" }},
		{ "lighting",	{ &lighting,		"set the lighting type of the device to <data>" }},
		{ "toggle",		{ &toggle,			"toggle the device in/out of flash mode" }},
		{ "watchdog",	{ &watchdog,		"set the watchdog timeout (ms)" }},
		{ "backup",		{ &backup_storage,	"backup storage from channel <data> to binary file" }},
		{ "restore",	{ &restore_storage,	"restore storage to channel <data> from binary file" }},
		{ "check",		{ &check_storage,	"check storage if the appropriate binary file exists" }},
		{ "read",		{ &read_storage,	"read storage from channel <data>" }},
		{ "reset",		{ &reset,			"force a reset of the device" }},
		{ "dump",		{ &dump,			"dump all of the current feature values from a camera" }},
		{ "bootstrap",	{ &bootstrap,		"bootstrap a new device from <data> configuration file" }}
	};


	if (!commands.count(cmd))
	{
		std::cout << std::endl << "Available commands:" << std::endl;

		for (auto &[k, v] : commands)
		{
			std::cout << "    " << std::setw(12) << k << "  " << v.description << std::endl;
		}

		return;
	}

	Flasc flasc([](double progress, string message) {

		if (progress < 0)			std::cout << "  " << message << std::endl;
		else if (message.empty())	std::cout << "." << std::flush;
		else						std::cout << "  " << message << std::flush;
	}, params.timeout);

	commands.at(cmd).operation(flasc, params);
}





int main(int argc, char *argv[])
{
	bool help		= false;
	bool version	= false;
	int timeout		= 500;
	string serial, id, cmd, data;

	emg::Clap clap;

	clap['h'].Name("help")		.Describe("display this help and exit")						.Bind(help);
	clap['v'].Name("version")	.Describe("display libpsinc version")						.Bind(version);
	clap['s'].Name("serial")	.Describe("connect to device with the given serial number")	.Bind(serial);
	clap['i'].Name("id")		.Describe("connect to device with the given ID")			.Bind(id);
	clap['c'].Name("command")	.Describe("command")										.Bind(cmd);
	clap['d'].Name("data")		.Describe("command specific data")							.Bind(data);
	clap['t'].Name("timeout")	.Describe("override USB timeout in ms (default is 500)")	.Bind(timeout);
	clap[1].Name("command")		.Describe("command")										.Bind(cmd);
	clap[2].Name("data")		.Describe("command specific data")							.Bind(data);

	emg::Log::Initialise({ std::make_unique<emg::logger::Console>() });
	// Log::Verbosity(Severity::Info);

	clap.Parse(argc, argv);

	if (help)
	{
		clap.Usage(std::cout, argv[0]);
		command("", {});
	}
	else if (version)
	{
		std::cout << "flasc version " << FLASC_VERSION " (built " __DATE__ " " __TIME__ ")" << std::endl;
		std::cout << "libpsinc version " << psinc::Version() << std::endl;
	}
	else
	{
		// command(cmd, serial, id, data, timeout);
		command(cmd, Params(timeout, id, serial, data));
	}

	return 0;
}
