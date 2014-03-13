using System;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using System.Threading;
using System.Reflection;
using System.Collections.Generic;


namespace libpsinc
{
	/// <summary>
	/// The primary interface to the library
	/// </summary>
	public class Camera : IDisposable
	{
		/// <summary>
		/// Occurs when an image acquisition is completed
		/// </summary>
		public event AcquiredEvent Acquired;	


		/// <summary>
		/// Occurs when the hardware connection status changes
		/// </summary>
		public event ConnectionEvent ConnectionChanged = delegate {};


		/// <summary>
		/// Occurs when data transfer from the capture device has failed
		/// </summary>
		public event TransferErrorEvent TransferError
		{
			add		{ this.transport.TransferError += value; }
			remove	{ this.transport.TransferError -= value; }
		}


		/// <summary>
		/// Gets or sets the image handler used to process data from the camera
		/// and convert it to the required image type. The initial value is the
		/// DefaultImageHandler which decodes the data to a System.Drawing.Bitmap.
		/// </summary>
		/// <value>The image handler.</value>
		public ImageHandler ImageHandler { get; set; }


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
		public bool Colour { get; set; }


		/// <summary>
		/// Gets or sets the flash power
		/// </summary>
		/// <value>The flash power</value>
		public byte Flash { get; set; }


		/// <summary>
		/// Gets or sets the sleep period of the acquisition thread.
		/// </summary>
		/// <remarks>
		/// The thread will sleep for this period every loop.
		/// Consider using the pause/resume functions where appropriate
		/// to your application rather than manipulating the sleep period.
		/// </remarks>
		/// <value>The sleep period in ms.</value>
		public int Sleep { get; set; }


		/// <summary>
		/// Gets or sets the capture mode of the camera.
		/// </summary>
		/// <remarks>
		/// Unless you are operating with multiple synchronised
		/// cameras, set this to CaptureMode.Normal. See the 
		/// CaptureMode enum for more information.
		/// </remarks>
		/// <value>The mode.</value>
		public CaptureMode Mode { get; set; }


		/// <summary>
		/// Gets the features available on the camera as a dictionary of <feature name, Feature>
		/// </summary>
		/// <value>The available features.</value>
		public SortedDictionary<string, Feature> Features { get; protected set; }
		
		
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
		public AliasCollection Aliases { get; protected set; }


		/// <summary>
		/// Gets the devices and virtual devices available through the connected camera.
		/// </summary>
		/// <remarks>
		/// Devices are peripheral devices which are connected to the camera and can be
		/// controlled through it. Examples of Devices include electronic locks, card
		/// readers and visual feedback decvices such as LEDs.
		/// </remarks>
		/// <value>The devices.</value>
		public Dictionary<string, Device> Devices { get; protected set; }
		
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="libpsinc.Camera"/> is connected
		/// to a hardware device.
		/// </summary>
		/// <value><c>true</c> if connected; otherwise, <c>false</c>.</value>
		public bool Connected
		{
			get { return this.transport.Connected; }
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
				return this.Aliases.context;
			}
			
			set
			{
				if (value < this.contextCount && this.Aliases["Context"] != null)
				{
					this.Aliases["Context"].Value = this.Aliases.context = value;
				}
			}
		}



		/// <summary>
		/// Exit flag - signals that the main acquisition thread is to 
		/// terminate
		/// </summary>
		protected bool exit = false;
		
		
		/// <summary>
		/// True if the camera has a monochrome imaging chip, otherwise false.
		/// </summary>
		protected bool monochrome = false;
		
		
		/// <summary>
		/// The acquisition thread.
		/// </summary>
		protected Thread thread = null;
		
		
		/// <summary>
		/// Pauses the acquisition thread if reset.
		/// </summary>
		protected ManualResetEvent pause = new ManualResetEvent(true);
		
		
		/// <summary>
		/// The transport layer of the camera
		/// </summary>
		Transport transport = new Transport();


		/// <summary>
		/// The control registers on the camera.
		/// </summary>
		List<Register> registers = new List<Register>();


		/// <summary>
		/// The number of control contexts available
		/// </summary>
		protected byte contextCount = 1;


		/// <summary>
		/// The default imaging chip type (if the camera reports back 
		/// no chip type) is asc_v024
		/// </summary>
		protected static string defaultType = "asc_v024";


		/// <summary>
		/// The default devices available on the camera if the camera
		/// reports back no device list is currently empty
		/// </summary>
		protected static byte[] defaultDevices = new byte [] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };


		/// <summary>
		/// Pool of all supported types of sub-Device (populated in the constructor)
		/// </summary>
		protected Dictionary<byte, Device> devicePool = null;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <remarks>Initialises the Camera to return
		/// greyscale images, operate in Normal (unsynchronised)
		/// capture mode, no flash and a sleep period of 1.
		/// <remarks>
		public Camera()
		{
			this.Colour			= false;
			this.Mode			= CaptureMode.Normal;
			this.ImageHandler	= new DefaultImageHandler();
			this.Flash			= 0;
			this.Sleep			= 1;
			this.Features		= new SortedDictionary<string, Feature>();
			this.Devices		= new Dictionary<string, Device>();
			this.Aliases		= new AliasCollection() { features = this.Features };
			
			// Notes:
			// The "Name" also appears as part of the serial in the USB descriptor for this device.
			this.devicePool = new Dictionary<byte, Device>() {
				{ 0x00, new Device(transport, "Prox", 		0x00, Device.DataDirection.Input) },							// Prox reader device
				{ 0x01, new Device(transport, "Lock",		0x01, Device.DataDirection.Output) },	// Electronic lock control
				{ 0x02, new Device(transport, "LEDArray",	0x02, Device.DataDirection.Output) },	// LED array
				{ 0x03, new Device(transport, "SecureLock",	0x03) },														// Encrypted lock control
				{ 0x04, new Device(transport, "Error", 		0x04, Device.DataDirection.Input) },							// Error reporting
				{ 0x05, new Device(transport, "Serial", 	0x05) },														// Serial number of the camera (16 bytes)
				{ 0x06, new Device(transport, "Storage0", 	0x06) },														// Storage block 0 (free for use - 502 bytes)
				{ 0x07, new Device(transport, "Name", 		0x07, Device.DataDirection.Both) },	// User-settable name of the camera.
				{ 0x08, new Device(transport, "Storage1", 	0x08) },														// Storage block 1 (free for use - 127 bytes)
				{ 0x09, new Device(transport, "Defaults", 	0x09) },														// Default settings for this device. Modify with care.
				{ 0x0e, new Device(transport, "LEDPair",	0x0e, Device.DataDirection.Output)},	// Simple LED pair
				{ 0xff, new Device(transport, "Query", 		0xff, Device.DataDirection.Input) }								// Query the camera for a list of available devices and chip type
			};

			this.transport.ConnectionChanged += this.OnConnection;
		}


		/// <summary>
		/// Releases all resources used by the <see cref="libpsinc.Camera"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.Camera"/>. The
		/// <see cref="Dispose"/> method leaves the <see cref="libpsinc.Camera"/> in an unusable state. After calling
		/// <see cref="Dispose"/>, you must release all references to the <see cref="libpsinc.Camera"/> so the garbage
		/// collector can reclaim the memory that the <see cref="libpsinc.Camera"/> was occupying.</remarks>
		public void Dispose()
		{
			this.Disconnect();
			this.transport.Dispose();
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

			this.thread = null;
		}


		/// <summary>
		/// Refreshes the local copy of the current feature values held by the camera.
		/// </summary>
		protected bool RefreshFeatures()
		{
			bool refreshed	= false;
			var receive		= new byte[512];
			
			foreach (byte page in this.registers.Select(r => r.Page).Distinct())
			{
				if (this.transport.Command(new byte [] { (byte)Commands.ReadRegisterPage, page }, receive))
				{
					foreach (var register in this.registers.Where(r => r.Page == page).Cast<Register>())
					{
						register.Refresh(receive);
					}
					
					refreshed = true;
				}
				else Console.WriteLine("Failed to refresh registers for page {0} when connecting", page);
			}
			
			if (refreshed) 
			{
				this.Aliases.context = (byte)this.Aliases["Context"].Value;
			};
			return refreshed;
		}
		
		
		/// <summary>
		/// Resets the connected camera.
		/// </summary>
		public void Reset()
		{
			this.transport.Reset();
		}
		
		
		/// <summary>
		/// Resets the imaging chip in the connected camera to its default state.
		/// </summary>
		public void ResetChip()
		{
			this.transport.Command(new byte [] { (byte)Commands.ResetChip }, null);
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
		/// Initialise the Camera to connect to a physical camera with the specified
		/// chip type and colour depth on the specified bus with a given camera serial number.
		/// 
		/// </summary>
		/// <param name="type">string describing the imaging chip present in the connected hardware</param>
		/// <param name="monochrome">If set to <c>true</c> the imaging chip in the hardware is monochrome, otherwise it is a bayer-colour chip.</param>
		/// <param name="supplementary">Supplementary XML defining any connected Devices.</param>
		/// <param name="serial">Serial number of the camera to connect to as a regular expression.
	 	/// If empty the Camera will connect to the first available camera on the specified bus.</param>
		/// <param name="bus">Bus ID to find the device on. If set to zero, the acquirer will look for the first device with a 
		/// matching serial on any bus.</param>
		public bool Initialise(string serial = "", uint bus = 0)
		{;
			this.transport.Serial	= serial;
			this.transport.Bus		= bus;

			if (this.thread == null)
			{
				this.exit	= false;
				this.thread	= new Thread(new ThreadStart(this.Entry));
				this.thread.Start();
				return true;
			}
			return false;
		}


		/// <summary>
		/// Entry point for the main acquisition thread.
		/// </summary>
		void Entry()
		{
			while (!this.exit)
			{
				if (this.Acquired != null)
				{
					object image = null;

					if (this.transport.Connected)
					{
						var colour	= this.monochrome ? ColourMode.Monochrome : this.Colour ? ColourMode.BayerColour : ColourMode.BayerGrey;
						image		= this.Capture(this.Mode, colour, this.Flash);
					}
					else this.transport.Initialise();

					this.Acquired(image);
				}

				Thread.Sleep(this.Sleep);

				if (!exit) this.pause.WaitOne();
			}
		}


		/// <summary>
		/// Configures the Camera based upon the embedded XML camera description
		/// </summary>
		/// <returns><c>true</c>, if the requested XML was found, <c>false</c> otherwise.</returns>
		/// <param name="description">XML description of the camera</param>
		protected bool ConfigureCamera(Stream description)
		{
			if (description != null)
			{
				var xml = XElement.Load(description);
				
				if (xml.Name == "camera")
				{
					this.contextCount = (byte)((int)xml.Attribute("contexts"));

					foreach (XElement r in xml.Elements("register"))
					{
						var register = new Register(this.transport, r);
						
						foreach (XElement f in r.Elements("feature"))
						{
							var feature = new Feature(f, register);
							
							this.Features.Add((string)f.Attribute("name"), feature);
						}
						
						this.registers.Add(register);
					}
					
					for (byte i=0; i<this.contextCount; i++)
					{
						this.Aliases.aliases[i] = xml.Elements("alias")
							.Where(a => a.Attribute("context") == null || (int)a.Attribute("context") == i)
								.ToDictionary(a => (string)a.Attribute("name"), a => (string)a.Attribute("feature"));
					}
					return true;
				}
			}
			return false;
		}


		/// <summary>
		/// When a connection event is raised, check if it is a connection or disconnection.
		/// If it is a connection, connect to and configure the connected camera.
		/// 
		/// Pass the connection status on to ConnectionChanged event.
		/// </summary>
		/// <param name="connected">If set to <c>true</c> a camera has been connected.</param>
		protected void OnConnection(bool connected)
		{
			if (connected)
			{
				this.Features		= new SortedDictionary<string, Feature>();
				this.Devices		= new Dictionary<string, Device>();
				this.Aliases		= new AliasCollection() { features = this.Features };
				byte[] devices		= Camera.defaultDevices;
				string cameraType	= Camera.defaultType;
				var description		= this.devicePool[0xFF].Read();

				if (description != null && description.Length > 0)
				{
					int header = description[0];

					if (header > 1)
					{
						switch (description[1])
						{
							//case 0x00:	cameraType = Camera.defaultType;	break;
							default:	cameraType = Camera.defaultType;	break;
						}

						this.monochrome = (description[2] & 0x01) > 0;
					}

					int count	= description[header + 1];
					devices		= description.Skip(header + 2).Take(count).ToArray();
				}

				foreach (var d in devices)
				{
					this.Devices.Add(this.devicePool[d].Name, this.devicePool[d]);
				}

				this.ConfigureCamera(Assembly.GetAssembly(this.GetType()).GetManifestResourceStream(cameraType));

				if (!this.RefreshFeatures())
				{
					this.transport.Release();
					return;
				}
			}

			this.ConnectionChanged(connected);
		}


		/// <summary>
		/// Capture an image
		/// </summary>
		/// <param name="mode">Capture mode</param>
		/// <param name="colour">Colour mode</param>
		/// <param name="flash">Flash power</param>
		protected object Capture(CaptureMode mode, ColourMode colour, byte flash)
		{
			int width		= (int)this.Aliases["Width"].Value;
			int height		= (int)this.Aliases["Height"].Value;
			int expected 	= width * height;
			byte [] receive	= new byte[expected];
			byte [] command	= { 
				CaptureCommands.Map[mode], 
				flash, 
				(byte)(expected & 0xff), 
				(byte)((expected >> 8) & 0xff), 
				(byte)((expected >> 16) & 0xff) 
			};
			
			if (this.transport.Command(command, receive) && this.ImageHandler != null)
			{
				return this.ImageHandler.Decode(colour, receive, width, height);
			}
			
			return null;
		}

	}
}
