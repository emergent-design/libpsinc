#include <iostream>
#include <psinc/flash/Flash.hpp>
#include <emergent/Clap.hpp>
#include <emergent/logger/Logger.hpp>


using namespace std;
using namespace emergent;
using namespace psinc::flash;


// USING LIBSERIALPORT ON WINDOWS
//
// At the time of writing the standard version of libserialport for Windows
// had issues with correctly obtaining the corresponding USB descriptor
// information for a USB-backed serial device [1]. The following describes
// a patch which allows it to work.
//
// Starting with the fork at https://github.com/arduino/libserialport
//
//   * Open the windows.c file.
//
//   * Towards the end of enumerate_hub_ports() change
//       if (fetchDescriptors) {
//     to
//       if (true) {
//
//   * In get_port_details() near the end, just after
//       port->usb_path = strdup(usb_path);
//     add the following
//       /* Wake up the USB device to be able to read string descriptor. */
//       char *escaped_port_name;
//       HANDLE handle;
//       if (!(escaped_port_name = malloc(strlen(port->name) + 5)))
//               RETURN_ERROR(SP_ERR_MEM, "Escaped port name malloc failed");
//       sprintf(escaped_port_name, "\\\\.\\%s", port->name);
//       handle = CreateFile(escaped_port_name, GENERIC_READ, 0, 0,
//                           OPEN_EXISTING,
//                           FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 0);
//       free(escaped_port_name);
//       CloseHandle(handle);
//
//   * Build the library following the standard instructions for libserialport.
//
// [1] https://github.com/martinling/libserialport/issues/12



// Set the UI lights to a random colour every 100ms
void disco(Flash &flash, uint8_t address)
{
	srand(time(0));
	while (true)
	{
		flash.Ui(address, rand() % 64, rand() % 64, rand() % 64);
		this_thread::sleep_for(100ms);
	}
}


// Display the "time on" information for each of the groups
void group(Flash &flash, uint8_t address)
{
	// flash.Group(0x0f, { 0x0800, 0x0800, 0x0800, 0x0800 });

	for (int i=0; i<0x10; i++)
	{
		for (auto &f : flash.Group(address, i))
		{
			cout << String::format("%04x\t", f);
		}
		cout << '\n';
	}
}


// Sit in a loop reading the device temperatures every 2s
void temperature(Flash &flash, uint8_t address)
{
	while (true)
	{
		auto temp = flash.Temperature(address);
		cout << String::format("Driver: %.2f°C    LED: %.2f°C", temp[0], temp[1]) << '\n';

		this_thread::sleep_for(2s);
	}
}


// Display basic information about the flash
void info(Flash &flash, uint8_t address)
{
	auto version = flash.Version(address);

	cout << "          ID: " << flash.ID(address) << '\n';
	cout << "    Hardware: v" << version[0] << '\n';
	cout << "    Firmware: v" << version[1] << '\n';
	cout << "LED Hardware: v" << flash.LedHardware(address) << '\n';
	cout << "      LED ID: " << flash.LedID(address) << '\n';
}


// Display the current power levels for the flash
void power(Flash &flash, uint8_t address)
{
	// flash.CurrentLevel(43);

	cout << "Current level: " << (int)flash.CurrentLevel(address) << '\n';
	cout << "Voltage level: " << (int)flash.VoltageLevel(address) << '\n';
	cout << "   Voltage in: " << flash.VoltageIn(address) << "V\n";
	cout << "  Voltage out: " << flash.VoltageOut(address) << "V\n";
}


// Display the timing configuration for continuous flash mode
void continuous(Flash &flash, uint8_t address)
{
	// flash.Continuous({ 0x1000, 0x1000, 0x1000, 0x1000 });

	for (auto &f : flash.Continuous(address))
	{
		cout << String::format("%04x\t", f);
	}
	cout << "\n";

	// flash.EnableContinuous(true);
}


// Change some settings and persist them
void save(Flash &flash, uint8_t address)
{
	flash.Ui(address, 0, 0, 0);
	flash.CurrentLevel(address, 43);
	flash.Save(address);
	this_thread::sleep_for(200ms);
}


// List all available serial devices
void list()
{
	for (auto &f : Flash::List())
	{
		cout << f.first << " : " << f.second << endl;
	}
}


int main(int argc, char *argv[])
{
	map<string, function<void(Flash&, uint8_t address)>> commands = {
		{ "find",			[](auto &f, auto a) {}},
		{ "list", 			[](auto &f, auto a) { list(); }},
		{ "off",			[](auto &f, auto a) { f.Ui(a, 0, 0, 0); }},
		{ "on",				[](auto &f, auto a) { f.Ui(a, 255, 0, 0); }},
		{ "info",			[](auto &f, auto a) { info(f, a); }},
		{ "temperature",	[](auto &f, auto a) { temperature(f, a); }},
		{ "disco",			[](auto &f, auto a) { disco(f, a); }},
		{ "group",			[](auto &f, auto a) { group(f, a); }},
		{ "power",			[](auto &f, auto a) { power(f, a); }},
		{ "continuous",		[](auto &f, auto a) { continuous(f, a); }}
	};

	// Display any log information generated by the Flash helper class
	Log::Initialise({ unique_ptr<logger::Sink>(new logger::Console()) });
	Log::Verbosity(Severity::Info);


	Clap clap;
	bool help = false;
	uint8_t address = 1;
	string connection;
	string command;

	clap['h'].Name("help").Describe("display this help information").Bind(help);
	clap['a'].Name("address").Describe("address of flash device (default=1)").Bind(address);
	clap[1].Name("command").Describe("command to execute").Bind(command);
	clap[2].Name("connection").Describe("serial port connection (path on linux, COM on windows)").Bind(connection);

	clap.Parse(argc, argv);

	if (help)
	{
		clap.Usage(cout, argv[0]);

		cout << "Commands" << endl;
		for (auto &c : commands)
		{
			cout << '\t' << c.first << endl;
		}

		return 0;
	}

	if (command.empty() || !commands.count(command))
	{
		cout << "Please specify a valid command, see help (-h) for a list of available commands" << endl;
		return 0;
	}

	if (command == "find")
	{
		cout << "Device with serial regex '" << connection << "' is " << Flash::Find(connection) << endl;
		return 0;
	}

	{
		Flash flash;

		if (command == "list" || flash.Initialise(connection))
		{
			this_thread::sleep_for(10ms);	// Give the connection log a chance to print
			commands[command](flash, address);
		}
		else cout << "Failed to initialise flash connection" << endl;
	}

	this_thread::sleep_for(100ms);

	return 0;
}
