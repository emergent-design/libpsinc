#include <iostream>
#include <psinc/Camera.h>
#include <emergent/String.hpp>

using namespace std;
using namespace emg;
using namespace psinc;


int main(int argc, char *argv[])
{
	cout << "This test application connects to the first camera it finds" << endl;
	cout << "and prints out some information about the device it connected to" << endl;

	Camera camera;

	// Connect to the first camera that you find
	camera.Initialise();

	// Wait for the camera to connect. In a real application
	// you probably don't want to sit sleeping in the main 
	// thread like this - using the callback function as shown
	// in https://github.com/emergent-design/libpsinc/wiki#camera
	// is a better idea.
	while (!camera.Connected()) this_thread::sleep_for(1ms);

	// The camera has a query device from which we can pull low-level
	// information about the imaging chip. Query is used internally
	// by libpsinc to figure out how to use the device, but we can 
	// also use it to take a look inside the camera.
	auto query = camera.devices["Query"].Read();

	// Output some bits and bobs from the query
	if (query.Size() && query[0] > 1)
	{
		cout << "Type:     " << (int)query[1] << endl;
		cout << "Format:   " << (query[2] & 0x01 ? "bayer" : "monochrome") << endl;
		cout << "Mode:     " << (query[2] & 0x80 ? "flash" : "normal") << endl;
	}
	else cout << "Type:     0" << endl;


	// We can also read a variety of properties without using the Query device
	// (probably more generally useful for application developers).
	// This shows how you can read from a device...
	cout << "Lighting: " << (int)camera.CustomDevice(0x0c).Read()[0] << endl;
	cout << "Serial:   " << camera.devices["Serial"].Read() << endl;
	cout << "ID:       " << String::trim(camera.devices["Name"].Read(), ':') << endl;
	cout << "Hardware: " << String::trim(camera.CustomDevice(0x0b).Read(), '\0') << endl;
	cout << "Firmware: " << String::trim(camera.CustomDevice(0x0f).Read(), '\0') << endl;
	cout << "Watchdog: " << camera.CustomDevice(0x0d).Read() << endl;
}
