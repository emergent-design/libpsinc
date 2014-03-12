using System;
using System.Drawing;


namespace libpsinc
{
	/// <summary>Image acquired event</summary>
	public delegate void AcquiredEvent(object image);

	/// <summary>Hardware connection status changed event</summary>
	public delegate void ConnectionEvent(bool connected);

	/// <summary>Data transfer error event</summary>
	public delegate void TransferErrorEvent(string error);


	/// <summary>Defines the capture modes available</summary>
	public enum CaptureMode
	{
		/// <summary>Normal capture mode; do not sync with any other cameras</summary>
		Normal,

		/// <summary>Master capture mode - issue a capture signal for slaved devices when capture occurs</summary>
		Master,

		/// <summary>Slave capture mode - perform capture on the rising edge of the sync signal provided by a master device</summary>
		SlaveRising,

		/// <summary>Slave capture mode - perform capture on the falling edge of the sync signal provided by a master device</summary>
		SlaveFalling
	}


	/// <summary>Colour mode the camera is operating in</summary>
	public enum ColourMode
	{
		/// <summary>Monochrome (greyscale) images</summary>
		Monochrome,

		/// <summary>Bayer image, decoded to greyscale</summary>
		BayerGrey,

		/// <summary>Bayer image, decoded to colour</summary>
		BayerColour
	}
}
