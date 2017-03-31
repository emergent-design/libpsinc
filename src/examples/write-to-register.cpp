#include <iostream>
#include <psinc/Camera.h>

using namespace std;
using namespace emg;
using namespace psinc;


int main(int argc, char *argv[])
{
	cout << "This test application connects to the first camera it finds" << endl;
	cout << "And changes the exposure" << endl;

	Camera camera;

	// Connect to the first camera that you find
	camera.Initialise();

	// Wait for the camera to connect. In a real application
	// you probably don't want to sit sleeping in the main 
	// thread like this - using the callback function as shown
	// in https://github.com/emergent-design/libpsinc/wiki#camera
	// is a better idea.
	while (!camera.Connected()) this_thread::sleep_for(1ms);

	// Common features can be set by alias (so you don't need to know which
	// register to adjust for a given imaging chip)
	
	cout << "Current exposure of context 0 is " << camera.aliases[0].exposure->Get();
	// Set the exposure of context 0 to 42
	camera.aliases[0].exposure->Set(42);
	
	// If you don't want to use an alias, you have to look up the actual register for the
	// specific imaging chip you're using. So, for example, in the VO24 "exposure" of context
	// 0 is handled by the feature A: Coarse Shutter Width Total...
	
	cout << "A: Coarse Shutter Width Total is " << camera.features["A: Coarse Shutter Width Total"].Get();
	// Set the exposure to 24
	camera.features["A: Coarse Shutter Width Total"].Set(24);
	// And read it back again
	cout << "A: Coarse Shutter Width Total is now " << camera.features["A: Coarse Shutter Width Total"].Get();
	// ...which is the same as the exposure, of course...
	cout << "Current exposure of context 0 is " << camera.aliases[0].exposure->Get();
	
}
