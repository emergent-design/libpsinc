using System;
using System.IO;
using System.Drawing;
using System.Xml.Linq;
using System.Threading;
using System.Reflection;
using System.Collections.Generic;


namespace libpsinc
{
	/// <summary>
	/// The primary interface to the library
	/// </summary>
	public class Acquirer : IDisposable
	{
		/// <summary>
		/// The default imaging chip type (if the camera reports back 
		/// no chip type) is asc_v024
		/// </summary>
		protected readonly string defaultType = "asc_v024";

		/// <summary>
		/// The default colour type of the camera (if the camera reports
		/// back no colour model) is colour.
		/// </summary>
		protected readonly bool defaultMonochrome = false;

		/// <summary>
		/// The default devices available on the camera if the camera
		/// reports back no device list is currently empty
		/// </summary>
		protected readonly XElement defaultDevices = XElement.Parse(	
		   @"<devices>
		        <device name=""Prox"" index=""0"" direction=""input"" />
		        <device name=""Lock"" index=""1"" direction=""output"" type=""integer"" />
		        <device name=""LED""  index=""2"" direction=""output"" type=""integer"" />
				<device name=""SecureLock"" index=""3"" direction=""both"" />
				<device name=""Error"" index=""4"" direction=""input"" />
		        <device name=""Serial"" index=""5"" direction=""both"" />
				<device name=""LensCorrection"" index=""6"" direction=""both"" />
				<device name=""Name"" index=""7"" direction=""both"" type=""string""/>
				<device name=""Storage"" index=""8"" direction=""both"" />
				<device name=""Defaults"" index=""9"" direction=""both"" />
			</devices>" );

		/// <summary>
		/// Occurs when an image acquisition is completed
		/// </summary>
		public event AcquiredEvent Acquired;	


		/// <summary>
		/// Occurs when the hardware connection status changes
		/// </summary>
		public event ConnectionEvent ConnectionChanged	= delegate {};


		/// <summary>
		/// Occurs when data transfer from the capture device has failed
		/// </summary>
		public event TransferErrorEvent TransferError	= delegate {};


		/// <summary>
		/// Occurs when the local cache of control register values has been 
		/// refreshed
		/// </summary>
		public event RefreshedEvent Refreshed			= delegate {};


		/// <summary>
		///	Gets or sets a value which determines whether the library will
		/// attempt to return colour or greyscale images. 
		/// </summary>
		/// <remarks>
		/// This is independent from the type of imaging chip, though colour 
		/// images cannot be produced by a camera with a monochrome imaging 
		/// chip. 
		/// 
		/// If the camera contains a colour chip and Colour is true, images
		/// will be decoded to a colour bitmap using the BayerColour decoder.
		/// 
		/// If the camera contains a colour chip and Colour is false, images
		/// will be decoded to a greyscale bitmap using the BayerGrey decoder.
		/// 
		/// If the camera contains a monochrome chip, images will be handled 
		/// as a greyscale bitmap regardless of Colour.
		/// </remarks>
		/// <value><c>true</c> if colour; otherwise, <c>false</c>.</value>
		public bool Colour				{ get; set; }


		/// <summary>
		/// Gets or sets the flash power
		/// </summary>
		/// <value>The flash power</value>
		public byte Flash				{ get; set; }


		/// <summary>
		/// Gets or sets the sleep period of the acquisition thread.
		/// </summary>
		/// <remarks>
		/// The thread will sleep for this period every loop.
		/// Consider using the pause/resume functions where appropriate
		/// to your application rather than manipulating the sleep period.
		/// </remarks>
		/// <value>The sleep period in ms.</value>
		public int Sleep				{ get; set; }


		/// <summary>
		/// Gets or sets the capture mode of the camera.
		/// </summary>
		/// <remarks>
		/// Unless you are operating with multiple synchronised
		/// cameras, set this to CaptureMode.Normal. See the 
		/// CaptureMode enum for more information.
		/// </remarks>
		/// <value>The mode.</value>
		public CaptureMode Mode 		{ get; set; }

		/// <summary>
		/// Exit flag - signals that the main acquisition thread is to 
		/// terminate
		/// </summary>
		bool exit				= false;


		/// <summary>
		/// True if the camera has a monochrome imaging chip, otherwise false.
		/// </summary>
		bool monochrome			= false;


		/// <summary>
		/// The acquisition thread.
		/// </summary>
		Thread thread			= null;


		/// <summary>
		/// The camera device.
		/// </summary>
		Camera camera			= null;


		/// <summary>
		/// Pauses the acquisition thread if reset.
		/// </summary>
		ManualResetEvent pause	= new ManualResetEvent(true);


		/// <summary>
		/// Constructor.
		/// </summary>
		/// <remarks>Initialises the Acquirer to return
		/// greyscale images, operate in Normal (unsynchronised)
		/// capture mode, no flash and a sleep period of 1.
		/// <remarks>
		public Acquirer()
		{
			this.Colour = false;
			this.Mode	= CaptureMode.Normal;
			this.Flash	= 0;
			this.Sleep	= 1;
		}


		/// <summary>
		/// Releases all resource used by the <see cref="libpsinc.Acquirer"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.Acquirer"/>. The
		/// <see cref="Dispose"/> method leaves the <see cref="libpsinc.Acquirer"/> in an unusable state. After calling
		/// <see cref="Dispose"/>, you must release all references to the <see cref="libpsinc.Acquirer"/> so the garbage
		/// collector can reclaim the memory that the <see cref="libpsinc.Acquirer"/> was occupying.</remarks>
		public void Dispose()
		{
			this.Disconnect();
		}


		/// <summary>
		/// Disconnect this instance from the physical capture device.
		/// </summary>
		public void Disconnect()
		{
			lock(this)
			{
				this.exit = true;	
				this.Resume();
			}

			if (this.thread != null) this.thread.Join();
			if (this.camera != null) this.camera.Dispose();

			this.thread = null;
			this.camera = null;
		}


		/// <summary>
		/// Gets the features available in the connected hardware.
		/// </summary>
		/// <value>The features (in a dictionary of [string FeatureName, Feature feature])</value>
		public SortedDictionary<string, Feature> Features 
		{
			get { return (this.camera == null) ? null : this.camera.Features; }
		}


		/// <summary>
		/// Gets the common feature aliases available.
		/// </summary>
		/// <remarks>
		/// Features commonly available on all supported imaging hardware have common aliases, 
		/// regardless of their feature name in the specific device. This allows devices to be 
		/// used to some extent without caring what specific type of camera is connected.
		/// 
		/// For example the Gain of the imaging chip can always be accessed through the
		/// AliasCollection.Gain feature regardless of the device-specific implementation of
		/// gain control.
		/// </remarks>
		/// <value>The aliases.</value>
		public AliasCollection Aliases
		{
			get { return (this.camera == null) ? null : this.camera.Aliases; }
		}
		

		/// <summary>
		/// Gets the devices and virtual devices available through the connected camera.
		/// </summary>
		/// <remarks>
		/// Devices are peripheral devices which are connected to the camera and can be
		/// controlled through it. Examples of Devices include electronic locks, card
		/// readers and visual feedback decvices such as LEDs.
		/// </remarks>
		/// <value>The devices.</value>
		public Dictionary<string, Device> Devices
		{
			get { return (this.camera == null) ? null : this.camera.Devices; }
		}
		

		/// <summary>
		/// Gets a value indicating whether this <see cref="libpsinc.Acquirer"/> is connected
		/// to a hardware device.
		/// </summary>
		/// <value><c>true</c> if connected; otherwise, <c>false</c>.</value>
		public bool Connected
		{
			get { return this.camera == null ? false : this.camera.Connected; }
		}


		/// <summary>
		/// Gets or sets the current camera context.
		/// </summary>
		/// <remarks>
		/// In terms of the camera, a context is a collection of
		/// control register settings. Any or all of the available
		/// registers or features can be set in a given context. By
		/// switching camera contexts, these settings can be used as
		/// predefined capture modes. The contexts are implemented
		/// in hardware, and are a feature of the imaging chip.
		/// 
		/// The number of available contexts is hardware dependent.
		/// </remarks>
		/// <value>The current camera context.</value>
		public byte Context
		{
			get 
			{
				return this.camera != null ? this.camera.Context : (byte)0;
			}

			set
			{
				if (this.camera != null) this.camera.Context = value;
			}
		}


		/// <summary>
		/// Initialise the Acquirer to connect to a camera with the specified
		/// chip type and colour depth on the specified bus with a given camera serial number.
		/// 
		/// </summary>
		/// <param name="type">string describing the imaging chip present in the connected hardware</param>
		/// <param name="monochrome">If set to <c>true</c> the imaging chip in the hardware is monochrome, otherwise it is a bayer-colour chip.</param>
		/// <param name="supplementary">Supplementary XML defining any connected Devices.</param>
		/// <param name="serial">Serial number of the camera to connect to as a regular expression.
	 	/// If empty the Acquirer will connect to the first available camera on the specified bus.</param>
		/// <param name="bus">Bus ID to find the device on. If set to zero, the acquirer will look for the first device with a 
		/// matching serial on any bus.</param>
		public bool Initialise(string serial = "", uint bus = 0)
		{
			bool result = false;

			if (this.camera == null)
			{
				//TODO: Query the camera to determine the chip type and supplementary devices it offers.
				//If the camera doesn't supply any of this information, use the standard defaults.
				//(for now, we just use the defaults)
				try
				{
					Stream xml = Assembly.GetAssembly(this.GetType()).GetManifestResourceStream(this.defaultType);		

					if (xml != null)
					{
						var root = XElement.Load(xml);
						if (this.defaultDevices != null) root.Add(this.defaultDevices.Elements());
	
						this.camera	= new Camera();

						if (this.camera.Initialise(root, serial, bus))
						{
							this.camera.ConnectionChanged	+= c => this.ConnectionChanged(c);
							this.camera.TransferError		+= e => this.TransferError(e);
							this.camera.Refreshed 			+= () => this.Refreshed();

							this.monochrome	= this.defaultMonochrome;
							this.exit		= false;
							this.thread		= new Thread(new ThreadStart(this.Entry));
							this.thread.Start();
							result = true;
						}
					}
				}
				catch (Exception) {}
		

				if (!result && this.camera != null)
				{
					this.camera.Dispose();
					this.camera = null;
				}
			}

			return result;
		}

		/// <summary>
		/// Refreshes the local copy of the current feature values held by the camera.
		/// </summary>
		public void RefreshFeatures()
		{
			if (this.camera != null && this.camera.Connected) this.camera.Refresh();
		}
		

		/// <summary>
		/// Resets the connected camera.
		/// </summary>
		public void Reset()
		{
			if (this.camera != null && this.camera.Connected) this.camera.Reset();
		}


		/// <summary>
		/// Resets the imaging chip in the connected camera to its default state.
		/// </summary>
		public void ResetChip()
		{
			if (this.camera != null && this.camera.Connected) this.camera.ResetChip();
		}
	

		/// <summary>
		/// Pause the acquisition thread. No further communication with the camera
		/// will be carried out until the acquisition thread is resumed.
		/// </summary>
		public void Pause()
		{
			lock (this)
			{
				if(!exit) this.pause.Reset();
			}
		}

		/// <summary>
		/// Resume the acquisition thread. Communications, if paused, will resume.
		/// </summary>
		public void Resume()
		{
			this.pause.Set();
		}

		/// <summary>
		/// Entry point for the main acquisition thread.
		/// </summary>
		void Entry()
		{
			while (!this.exit)
			{
				Bitmap image = null;

				if (this.camera != null && this.Acquired != null)
				{
					if (this.camera.Connected)
					{
						var colour	= this.monochrome ? ColourMode.Monochrome : this.Colour ? ColourMode.BayerColour : ColourMode.BayerGrey;
						image		= this.camera.Capture(this.Mode, colour, this.Flash);
					}
					else this.camera.Connect();
				}

				if (this.Acquired != null) this.Acquired(image);
				Thread.Sleep(this.Sleep);
				if(!exit)
				{
					this.pause.WaitOne();
				}
			}
		}
	}
}
