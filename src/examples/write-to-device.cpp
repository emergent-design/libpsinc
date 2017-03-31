#include <iostream>
#include <psinc/Camera.h>

using namespace std;
using namespace emg;
using namespace psinc;


int main(int argc, char *argv[])
{
	cout << "This test application connects to the first camera it finds" << endl;
	cout << "Reads the name, then sets it to the name passed in to this application" << endl;
	cout << "This prepends a : character to the name for ease of regex matching when" << endl;
	cout << "Connecting to cameras in later use" << endl;

	string newName = (string)":" + (argc > 1 ? argv[1] : "default");
	
	cout << "I want to call the camera " << newName << endl;
	
	Camera camera;

	// Connect to the first camera that you find
	camera.Initialise();

	// Wait for the camera to connect. In a real application
	// you probably don't want to sit sleeping in the main 
	// thread like this - using the callback function as shown
	// in https://github.com/emergent-design/libpsinc/wiki#camera
	// is a better idea.
	while (!camera.Connected()) this_thread::sleep_for(1ms);

	cout << "It's currently called " << camera.devices["Name"].Read() << endl;
	
	// Device writes (and reads) are synchronous so no need to wait or use
	// a callback.
	camera.devices["Name"].Write(newName);
	
	cout << "Now it's called " << camera.devices["Name"].Read() << endl;
}
