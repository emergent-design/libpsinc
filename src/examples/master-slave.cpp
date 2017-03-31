#include <iostream>
#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>

using namespace std;
using namespace emg;
using namespace psinc;


int main(int argc, char *argv[])
{
	cout << "This test application connects to two cameras by regex." << endl;
	cout << "The first is the master. The second is the slave." << endl;
	cout << "They must be physically wired up correctly to do slave capture!" << endl;
	
	if (argc == 3)
	{
		string masterName = argv[1];
		string slaveName = argv[2];
		
		Camera master, slave;

		master.Initialise(masterName);
		slave.Initialise(slaveName);
		
		//Always use a distinct image driver for each camera!
		ImageHandler<byte> masterHandler;
		ImageHandler<byte> slaveHandler;
		
		Image<byte> masterImage, slaveImage;
		
		masterHandler.Initialise(masterImage);
		slaveHandler.Initialise(slaveImage);

		// Wait for the camera to connect. In a real application
		// you probably don't want to sit sleeping in the main 
		// thread like this - using the callback function as shown
		// in https://github.com/emergent-design/libpsinc/wiki#camera
		// is a better idea.
		while (!master.Connected()) this_thread::sleep_for(1ms);
		while (!slave.Connected()) this_thread::sleep_for(1ms);
		
		// Set a grab going from the slave camera first; it will wait for a sync
		// pulse from the master before grabbing.
		
		slave.GrabImage(Camera::Mode::SlaveRising, slaveHandler, [&](bool status) {
			if (status)
			{	

				slaveImage.Save("slave.png");
			}
			//Don't want any more images, so return false.
			return false;
		});
		
		// Wait for the slave camera to be ready and waiting for the sync signal
		while(!slaveHandler.waiting) this_thread::sleep_for(1ms);
		
		// Issue a capture command to the master camera. This will emit a sync signal
		// to the slave and both will capture at the same time.
		master.GrabImage(Camera::Mode::Master, masterHandler, [&](bool status) {
			if (status)
			{	
				masterImage.Save("master.png");
			}
			//Don't want any more images, so return false.
			return false;
		});		
		
		// Wait for everything to finish
		while (master.Grabbing()) this_thread::sleep_for(1ms);
		while (slave.Grabbing()) this_thread::sleep_for(1ms);
	}
}