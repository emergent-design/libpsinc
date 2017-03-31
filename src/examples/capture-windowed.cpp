#include <iostream>
#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>

using namespace std;
using namespace emg;
using namespace psinc;

void Capture(Camera &camera, ImageHandler<byte> &handler, const string &filename)
{
	// An image of the type we wish to capture
	Image<byte> image;
	// Give the target image to the handler
	handler.Initialise(image);

	// Set an asynchronous grab going
	camera.GrabImage(Camera::Mode::Normal, handler, [&](bool status) {
		if (status)
    	{	
			// Grabbing was successful so do something with the image
			// Saving the image out inside the callback lambda isn't 
			// something you'd normally do here - process it, double
			// buffer it and then save the buffer to disk in a different
			// thread
			image.Save(filename);
		}
		//Don't want any more images, so return false.
		return false;
	});
	
	// Need to make sure we've finished grabbing before re-using the camera
	// or exiting the program (as the grab call is asynchronous and we're 
	// calling it from a trivial single-thread application)
	while (camera.Grabbing()) this_thread::sleep_for(1ms);

}

int main(int argc, char *argv[])
{
	cout << "This test application connects to a camera and sets up" << endl;
	cout << "two contexts with different windows" << endl;
	
	Camera camera;
	
	// The image handler that will be decoding the raw image for byte images
	ImageHandler<byte> handler;

	// Connect to the first camera that you find
	camera.Initialise();

	// Wait for the camera to connect. In a real application
	// you probably don't want to sit sleeping in the main 
	// thread like this - using the callback function as shown
	// in https://github.com/emergent-design/libpsinc/wiki#camera
	// is a better idea.
	while (!camera.Connected()) this_thread::sleep_for(1ms);
	
	//Set one window for context 0 at x = 32, y = 32, width and height = 128
	camera.SetWindow(0, 32, 32, 128, 128);
	
	//Set another window for context 1 at 256, 0, width 32, height 256;
	camera.SetWindow(1, 256, 0, 32, 256);
	
	//Set the two contexts to different exposures
	camera.aliases[0].exposure->Set(42);
	camera.aliases[1].exposure->Set(24);
	
	//Capture from one context
	camera.SetContext(1);
	Capture(camera, handler, "context1.png");
	
	//Capture from the other context
	camera.SetContext(0);
	Capture(camera, handler, "context0.png");
	
}