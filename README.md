libpsinc
========

libpsinc is a cross-platform acquisition driver for the [Perception-SI Ltd](http://www.psi-ltd.com) range of cameras. It provides a C++14 implementation that we test on Linux and Windows, but are unable to support Mac (it may work, but we have no way of checking). The core functionality is also wrapped by a C API allowing for easier interop with languages such as C#. An example
C# wrapper library has also been included that should work with both .NET and mono.

libpsinc relies upon [libusb](http://libusb.info/); see the libpsinc [installation instructions](https://github.com/emergent-design/libpsinc/wiki/Installation) for your platform.

More detailed usage documentation can be found on the [wiki](https://github.com/emergent-design/libpsinc/wiki).


# License #

libpsinc is freely available under the terms of the [MIT License](http://opensource.org/licenses/mit-license.html). You can use libpsinc in both proprietary and open-source applications; simply retain the copyright and permission notice from the [license](https://github.com/emergent-design/libpsinc/blob/master/LICENSE) in your source and add the following acknowledgement to your product distribution:

```
This software is based on libpsinc (https://github.com/emergent-design/libpsinc).
Libpsinc is Copyright (C) 2014-2016 Emergent Design
```


# Using libpsinc #

The main C++ interface is through the Camera class:

```cpp
#include <iostream>
#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>

using namespace std;
using namespace emg;
using namespace psinc;

// A simple synchronous example that will capture a single image
int main(int argc, char *argv[])
{
    Camera camera;
    Image<byte> image;
    ImageHandler<byte> handler(image);

    // Default initialisation will attempt to connect to any appropriate
    // camera and will register no callbacks.
    camera.Initialise();

    // Wait for a camera to be connected
    while (!camera.Connected())
    {
        this_thread::sleep_for(1ms);
    }

    // Grab an image - returns immediately
    camera.GrabImage(Camera::Mode::Master, handler, [](bool) { return false; });

    // Wait for the camera to finish grabbing
    while (camera.Grabbing())
    {
        this_thread::sleep_for(1ms);
    }

    // Save the image to disk
    image.Save("grab.png");
}
```


```cpp
#include <iostream>
#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>
#include <emergent/logger/Logger.hpp>

using namespace std;
using namespace emg;
using namespace psinc;

// Another example taking advantage of the asynchronous
// nature of the camera class to continuously capture images.
int main(int argc, char *argv[])
{
    Log::Initialise({ unique_ptr<logger::Sink>(new logger::Console()) });

    Camera camera;
    Image<byte> image;
    ImageHandler<byte> handler(image);

    // Initialise the camera with a callback function for handling connection events.
    camera.Initialise("", [](bool connected) {

        // The connected flag indicates the event type.
        cout << "Camera has been " << (connected ? "connected" : "disconnected") << endl;

        // Due to threading constraints on the callbacks it is not safe to
        // call GrabImage from here.
    });

    // The supplied callback function will be invoked when an attempted image grab
    // has completed. If successful then the Image<> controlled by the ImageHandler
    // will contain the new data. If in streaming mode then you must either copy
    // or process this image before returning true otherwise the next frame will
    // overwrite it.
    camera.GrabImage(Camera::Mode::Master, handler, [](bool status) {

        // The status flag indicates whether or not image grabbing was successful.
        cout << (status ? '.' : 'x') << flush;

        // Returning true indicates that another frame is required (streaming mode).
        return true;
    });

    while (true)
    {
        this_thread::sleep_for(100ms);
    }
}
```

Both of the above examples can be compiled using a suitable version of Clang/GCC with the options ```-std=c++14 -lpsinc -lfreeimage -lpthread```


## Features ##

Features are properties of the camera that you can either read or control such as A/D reference voltage or gain. These are exposed by libpsinc rather than giving direct access to the control registers of the camera as the registers can contain combined features in an array of interesting ways. Using the Feature class instead means you don't have to worry about this.


## Devices ##

...Or more correctly sub-devices. The PSI camera can control or receive data from further internal and external devices which have been connected to it such as LED arrays, prox card readers and electronic locks. The Device class provides raw access to them, but each device type may
vary in communication protocol. Please refer to the [wiki](https://github.com/emergent-design/libpsinc/wiki) for more information.
