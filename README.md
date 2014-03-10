libpsinc
========

libpsinc is a cross-platform acquisition driver for the [Perception-SI Ltd](http://www.psi-ltd.com) range of cameras. It provides a c# implementation, suitable for use on both .net and mono. A c++ implementation will be released shortly.

libpsinc relies upon [libusb](http://libusb.info/).

# Using libpsinc #

The main interface with libpsinc is through the Acquirer class:

```csharp
public class Example
{
  Acquirer camera = new Acquirer();
  
  public Example()
  {
    //First of all hook up all the event handlers...

    //This is going to handle incoming images
    this.camera.Acquired += this.OnCapture;

    //If you want to do something when the camera connection
    //state changed hook into ConnectionChanged
    this.camera.ConnectionChanged += this.OnConnection;

    //Typically you'd just log transfer errors, which you can do
    //(along with any other error reporting you need) from the
    //TransferError event
    this.camera.TransferError += this.OnError;

    //Acquirer uses a local cache of the camera settings. If you
    //request an update of your cache, this event will fire to 
    //let you know it's happened.
    this.camera.Refreshed += this.OnRefreshed;


    //Now initialise the camera. This will connect to the first
    //camera it can find, but you can also pass in a bus index
    //and serial number regex if you want to connect to a 
    //specific camera.
    if (this.camera.Initialise()
    {
	//now setup this.camera.Features. Take a look in the xml file for the camera
	//you're using for a list of the available features.
    }
    else Console.WriteLine("Failed to initialise camera");
  }

  protected void OnCapture(Bitmap image)
  {
    //image contains the frame you just captured.
    //Do with it what you will.
  }

} 
```

Remember to dispose the Acquirer when you've finished with it. Acquirer runs a capture thread constantly once it's initialised. If you don't wish to stream data all the time, simply pause the Acquirer and resume it when you want to capture another frame.

## Features ##

Features are properties of the camera that you can either read or control - A to D reference voltage, for example, or gain. These are exposed by libpsinc rather than giving direct access to the control registers of the camera as the registers can contain combined features in an array of interesting ways. Using the Features class instead means you don't have to worry about this.

## Devices ##

...Or more correctly sub-devices. The PSI camera can control or recieve data from further devices which have been connected to it such as LED arrays, prox card readers and electronic locks. These are represented in code using the Device class.
