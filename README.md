libpsinc
========

libpsinc is a cross-platform acquisition driver for the [Perception-SI Ltd](http://www.psi-ltd.com) range of cameras. It provides a c++ implementation and a c# implementation, suitable for use on both .net and mono.We test libpsinc on Linux and Windows, but are unable to support Mac (it may work, we have no way of checking).

libpsinc relies upon [libusb](http://libusb.info/); see the libpsinc [installation instructions](https://github.com/emergent-design/libpsinc/wiki/Installation) for your platform.

More detailed usage documentation can be found on the [wiki](https://github.com/emergent-design/libpsinc/wiki)

# License #

libpsinc is freely available under the terms of the [MIT License](http://opensource.org/licenses/mit-license.html). You can use libpsinc in both proprietary and open-source applications; simply retain the copyright and permission notice from the [license](https://github.com/emergent-design/libpsinc/blob/master/LICENSE) in your source and add the following acknowledgement to your product distribution:

```
This software is based on libpsinc (https://github.com/emergent-design/libpsinc).
Libpsinc is Copyright (C) 2014 Emergent Design
```


# Using libpsinc #

The main C# interface is through the Camera class:

```csharp
using libpsinc;

public class Example
{
  Camera camera = new Camera();

  public Example()
  {
    //First of all hook up all the event handlers...

    //This is going to handle incoming images
    this.camera.Acquired += i => this.OnCapture(i as Bitmap);

    //If you want to do something when the camera connection
    //state changed hook into ConnectionChanged
    this.camera.ConnectionChanged += this.OnConnection;

    //Typically you'd just log transfer errors, which you can do
    //(along with any other error reporting you need) from the
    //TransferError event
    this.camera.TransferError += this.OnError;

    //Now initialise the camera. This will connect to the first
    //camera it can find, but you can also pass in a bus index
    //and serial number regex if you want to connect to a
    //specific camera.
    if (this.camera.Initialise()
    {
        //The camera will now try to initialise itself, connect
        //and refresh the local cache of features. You'll know
        //when the camera is ready to use when the OnConnection
        //event fires...
    }
    else Console.WriteLine("Failed to initialise camera");
  }

  protected void OnConnection(bool connected)
  {
    if (connected)
    {
        //The camera just got connected.
        //You can now check what features and
        //devices are available through it and
        //what their current values are.
        //You're also now able to take control
        //of the camera settings...
    }
    else
    {
        //The camera just got unplugged.
        //Do whatever is appropriate in
        //your application whan that event
        //occurs.
    }
  }

  protected void OnCapture(Bitmap image)
  {
    //image contains the frame you just captured.
    //Do with it what you will.
  }

  protected void OnError(string error)
  {
    //A transfer error occurred. If you're interested
    //in seeing what it was, log or output the error.
  }

}
```

Remember to dispose the Camera when you've finished with it. Camera runs a capture thread constantly once it's initialised. If you don't wish to stream data all the time, simply pause the Camera and resume it when you want to capture another frame.

## Features ##

Features are properties of the camera that you can either read or control - A to D reference voltage, for example, or gain. These are exposed by libpsinc rather than giving direct access to the control registers of the camera as the registers can contain combined features in an array of interesting ways. Using the Features class instead means you don't have to worry about this.

## Devices ##

...Or more correctly sub-devices. The PSI camera can control or receive data from further devices which have been connected to it such as LED arrays, prox card readers and electronic locks. These are represented in code using the Device class.
