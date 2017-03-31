#include <iostream>
#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>

using namespace std;
using namespace emg;
using namespace psinc;

void CaptureMonoImage(Camera &camera, ImageHandler<byte> &handler)
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
			image.Save("mono-capture.png");
		}
		//Don't want any more images, so return false.
		return false;
	});
	
	// Need to make sure we've finished grabbing before re-using the camera
	// or exiting the program (as the grab call is asynchronous and we're 
	// calling it from a trivial single-thread application)
	while (camera.Grabbing()) this_thread::sleep_for(1ms);

}


void CaptureColourImage(Camera &camera, ImageHandler<byte> &handler)
{
	// An image of the type we wish to capture
	Image<byte, emg::rgb> image;
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
			image.Save("colour-capture.png");
		}
		//Don't want any more images, so return false.
		return false;
	});

	// Need to make sure we've finished grabbing before re-using the camera
	// or exiting the program (as the grab call is asynchronous and we're 
	// calling it from a trivial single-thread application)
	while (camera.Grabbing()) this_thread::sleep_for(1ms);
}


void CaptureHdrMonoImage(Camera &camera, ImageHandler<uint16_t> &handler)
{
	// An image of the type we wish to capture
	Image<uint16_t> image;
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
			image.Save("hdr-mono-capture.png");
		}
		//Don't want any more images, so return false.
		return false;
	});

	// Need to make sure we've finished grabbing before re-using the camera
	// or exiting the program (as the grab call is asynchronous and we're 
	// calling it from a trivial single-thread application)
	while (camera.Grabbing()) this_thread::sleep_for(1ms);
}


void CaptureHdrColourImage(Camera &camera, ImageHandler<uint16_t> &handler)
{
	// An image of the type we wish to capture
	Image<uint16_t, emg::rgb> image;
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
			image.Save("hdr-colout-capture.png");
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
	cout << "This test application connects to the first camera it finds" << endl;
	cout << "and captures various types of image" << endl;

	Camera camera;

	// The image handler that will be decoding the raw image for byte images
	ImageHandler<byte> handler;
        
	// The image handler that will be decoding HDR images
	ImageHandler<uint16_t> hdrHandler;

	// Connect to the first camera that you find
	camera.Initialise();

	// Wait for the camera to connect. In a real application
	// you probably don't want to sit sleeping in the main 
	// thread like this - using the callback function as shown
	// in https://github.com/emergent-design/libpsinc/wiki#camera
	// is a better idea.
	while (!camera.Connected()) this_thread::sleep_for(1ms);
        
	//Use the camera and normal image handler for byte images
	CaptureMonoImage(camera, handler);
	CaptureColourImage(camera, handler);

	//Use the camera and HDR image handler for hdr images
	CaptureHdrMonoImage(camera, hdrHandler);
	CaptureHdrColourImage(camera, hdrHandler);
        
}
