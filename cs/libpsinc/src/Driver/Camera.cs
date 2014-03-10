using System;
using System.Linq;
using System.Drawing;
using System.Xml.Linq;
using System.Collections.Generic;


namespace libpsinc
{
	/// <summary>
	/// Available commands
	/// </summary>
	internal enum Commands : byte
	{
		/// <summary>
		/// Perform an image capture.
		/// </summary>
		Capture				= 0x00,

		/// <summary>
		/// Perform image capture as a slave device on
		/// the rising edge of the sync signal
		/// </summary>
		SlaveCaptureRising	= 0x01,

		/// <summary>
		/// Reset the imaging chip
		/// </summary>
		ResetChip			= 0x02,

		/// <summary>
		/// Flush the device; return responses to all 
		/// commands queued in this and previous un-flushed
		/// commands.
		/// </summary>
		Flush				= 0x03,

		/// <summary>
		/// Capture an image and signal all waiting slaves to 
		/// perform a capture.
		/// </summary>
		MasterCapture		= 0x04,

		/// <summary>
		/// Perform image capture as a slave device on
		/// the falling edge of the sync signal
		/// </summary>
		SlaveCaptureFalling	= 0x05,

		/// <summary>
		/// Write a register
		/// </summary>
		WriteRegister		= 0x10,

		/// <summary>
		/// Write a bit
		/// </summary>
		WriteBit			= 0x11,

		/// <summary>
		/// Queue a register read
		/// </summary>
		QueueRegister		= 0x12,

		/// <summary>
		/// Read a page of registers
		/// </summary>
		ReadRegisterPage	= 0x13,

		/// <summary>
		/// Write to a sub-device
		/// </summary>
		WriteDevice			= 0x20,

		/// <summary>
		/// Queue a sub-device read.
		/// </summary>
		QueueDevice			= 0x21,

		/// <summary>
		/// Initialise a sub-device.
		/// </summary>
		InitialiseDevice	= 0x22,

		/// <summary>
		/// Block write to a sub-device.
		/// </summary>
		WriteDeviceBlock	= 0x23
		
	}

	/// <summary>
	/// A PSI camera
	/// </summary>
	class Camera
	{
		/// <summary>
		/// Gets the features available on the camera as a dictionary of <feature name, Feature>
		/// </summary>
		/// <value>The available features.</value>
		public SortedDictionary<string, Feature> Features	{ get; private set; }

		/// <summary>
		/// Gets the available sub-devices connected to the camera as a dictionary of 
		/// <device name, Device>
		/// </summary>
		/// <value>The available devices.</value>
		public Dictionary<string, Device> Devices			{ get; private set; }

		/// <summary>
		/// Gets the available common feature aliases.
		/// </summary>
		/// <value>The available aliases.</value>
		public AliasCollection Aliases						{ get; private set; }

		/// <summary>
		/// Gets the serial number of the camera
		/// </summary>
		/// <value>The serial number.</value>
		public string Serial 								{ get; private set; }

		/// <summary>
		/// Gets the bus the camera is connected to.
		/// </summary>
		/// <value>The bus.</value>
		public uint Bus		 								{ get; private set; }

		/// <summary>
		/// The transport layer of the camera
		/// </summary>
		protected Transport transport 			= new Transport();

		/// <summary>
		/// The control registers on the camera.
		/// </summary>
		protected List<Register> registers		= new List<Register>();

		/// <summary>
		/// Initialisation state of the camera.
		/// </summary>
		protected bool initialised				= false;

		/// <summary>
		/// The number of control contexts available
		/// </summary>
		byte contextCount						= 1;


		/// <summary>
		/// Occurs when the cache of control registers has been
		 /// refreshed.
		/// </summary>
		public event RefreshedEvent Refreshed	= delegate {};


		/// <summary>
		/// Occurs when connection status has changed.
		/// </summary>
		public event ConnectionEvent ConnectionChanged
		{
			add		{ this.transport.ConnectionChanged += value; }
			remove	{ this.transport.ConnectionChanged -= value; }
		}


		/// <summary>
		/// Occurs on a data transfer error.
		/// </summary>
		public event TransferErrorEvent TransferError
		{
			add		{ this.transport.TransferError += value; }
			remove	{ this.transport.TransferError -= value; }
		}


		/// <summary>
		/// Available capture modes, enum to raw instruction dictionary
		/// </summary>
		static readonly Dictionary<CaptureMode, byte> COMMANDS = new Dictionary<CaptureMode, byte> {
			{ CaptureMode.Normal,		(byte)Commands.Capture },
			{ CaptureMode.Master,		(byte)Commands.MasterCapture },
			{ CaptureMode.SlaveRising,	(byte)Commands.SlaveCaptureRising },
			{ CaptureMode.SlaveFalling,	(byte)Commands.SlaveCaptureFalling }
		};


		/// <summary>
		/// Initialise the camera with the specified serial number on the specified bus
		/// with the connected sub-devices described in the provided xml.
		/// </summary>
		/// <param name="xml">Xml description of the connected sub-devices.</param>
		/// <param name="serial">Serial number of the camera to initialise.</param>
		/// <param name="bus">Bus the camera is connected to.</param>
		public bool Initialise(XElement xml, string serial, uint bus)
		{
			if (!this.initialised && xml.Name == "camera")
			{
				this.Features		= new SortedDictionary<string, Feature>();
				this.Devices		= new Dictionary<string, Device>();
				this.Aliases		= new AliasCollection() { features = this.Features };
				this.contextCount	= (byte)((int)xml.Attribute("contexts"));
				
				foreach (XElement d in xml.Elements("device"))
				{
					var device = new Device(this.transport, d);
					
					if (device != null) this.Devices.Add((string)d.Attribute("name"), device);
				}
				
				foreach (XElement r in xml.Elements("register"))
				{
					var register = new Register(this.transport, r);
					
					foreach (XElement f in r.Elements("feature"))
					{
						var feature = new Feature(f, register);
						
						feature.Reset();
						
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

				this.Serial = serial;
				this.Bus = bus;

				this.initialised = true;
			}
			
			return this.initialised;
		}
		
		
		/// <summary>
		/// Releases all resource used by the <see cref="libpsinc.Camera"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.Camera"/>. The
		/// <see cref="Dispose"/> method leaves the <see cref="libpsinc.Camera"/> in an unusable state. After calling
		/// <see cref="Dispose"/>, you must release all references to the <see cref="libpsinc.Camera"/> so the garbage
		/// collector can reclaim the memory that the <see cref="libpsinc.Camera"/> was occupying.</remarks>
		public void Dispose()
		{
			this.transport.Dispose();
		}
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="libpsinc.Camera"/> is connected.
		/// </summary>
		/// <value><c>true</c> if connected; otherwise, <c>false</c>.</value>
		public bool Connected
		{
			get { return this.transport.Connected; }
		}
		
		/// <summary>
		/// Connect to the camera.
		/// </summary>
		public bool Connect()
		{
			if (!this.transport.Connected)
			{
				try
				{
					if (this.transport.Initialise(this.Serial, this.Bus)) 
					{
						this.Refresh();
						return true;
					}
				}
				catch (Exception)
				{
					return false;
				}
			}
			
			return false;
		}
		

		/// <summary>
		/// Gets or sets the current camera control context.
		/// </summary>
		/// <value>The current context.</value>
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
		/// Capture an image
		/// </summary>
		/// <param name="mode">Capture mode</param>
		/// <param name="colour">Colour mode</param>
		/// <param name="flash">Flash power</param>
		public Bitmap Capture(CaptureMode mode, ColourMode colour, byte flash)
		{
			int width		= (int)this.Aliases["Width"].Value;
			int height		= (int)this.Aliases["Height"].Value;
			int expected 	= width * height;
			byte [] receive	= new byte[expected];
			byte [] command	= { COMMANDS[mode], flash, (byte)(expected & 0xff), (byte)((expected >> 8) & 0xff), (byte)((expected >> 16) & 0xff) };
			
			if (this.transport.Command(command, receive))
			{
				return ImageHandler.Decode(colour, receive, width, height);
			}
			
			return null;
		}
		
		
		/// <summary>
		/// Refresh the local control register cache with values
		/// read from the physical camera.
		/// </summary>
		public void Refresh()
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
				this.Refreshed();
			}
		}

		/// <summary>
		/// Reset the camera.
		/// </summary>
		public bool Reset()
		{
			return this.transport.Reset();
		}

		/// <summary>
		/// Resets the imaging chip on the camera.
		/// </summary>
		public bool ResetChip()
		{
			return this.transport.Command(new byte [] { (byte)Commands.ResetChip }, null);
		}
	}
}
