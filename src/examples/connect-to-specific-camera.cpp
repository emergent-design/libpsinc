#include <iostream>
#include <psinc/Camera.h>
#include <emergent/String.hpp>

using namespace std;
using namespace emg;
using namespace psinc;


int main(int argc, char *argv[])
{
	cout << "This test application connects to a specific camera" << endl;
	cout << "Make sure you pass in a valid regular expression" << endl;
	cout << "We match against the id string inside the camera which is formed from" << endl;
	cout << "<serialnumber><name> where <serialnumber> is factory set. You can" <<endl;
	cout << "set name yourself - we recommend always beginning names with a : character"<<endl;
	cout << "for ease of regex matching in later use" << endl;

	string name = argc > 1 ? argv[1] : ".*";
	
	cout << "I want to connect to the first camera matching the regex " << name << endl;
	
	Camera camera;

	// Connect to the first matching camera that you find
	camera.Initialise(name);

	// Wait for the camera to connect. In a real application
	// you probably don't want to sit sleeping in the main 
	// thread like this - using the callback function as shown
	// in https://github.com/emergent-design/libpsinc/wiki#camera
	// is a better idea.
	while (!camera.Connected()) this_thread::sleep_for(1ms);
	
	cout << "Connected to camera " << camera.devices["Name"].Read() << endl;
}