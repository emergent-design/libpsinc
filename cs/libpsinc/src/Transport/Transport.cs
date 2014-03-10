using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;


namespace libpsinc
{
	/// <summary>
	/// Transport layer for the camera.
	/// Handles the nuts and bolts of actually sending and receiving data.
	/// </summary>
	internal class Transport : IDisposable
	{
		/// <summary>
		/// Occurs when connection status of the hardware has changed.
		/// </summary>
		public event ConnectionEvent ConnectionChanged	= delegate {};

		/// <summary>
		/// Occurs on a data transfer error.
		/// </summary>
		public event TransferErrorEvent TransferError	= delegate {};


		const ushort VENDOR				= 0x0525;
		const ushort PRODUCT			= 0xaaca;
		const byte WRITE_PIPE			= 0x03;
		const byte READ_PIPE			= 0x81;
		const int MAX_TRANSFER_ATTEMPTS = 4;
		const int TIMEOUT				= 200;

		IntPtr handle					= IntPtr.Zero;
		bool connected					= false;
		bool raiseError					= false;
		string lastError				= null;
		string serial;
		uint bus						= 0;
		byte[] buffer			 	 	= new byte[64];


		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.Transport"/> class.
		/// </summary>
		public Transport()
		{
			if (Environment.OSVersion.Platform != PlatformID.Unix)
			{
				if (!Windows.Instance.Loaded) 
				{
					throw new DllNotFoundException("Failed to load the libusb native library");
				}
			}

			unsafe { Usb.Init(null); }
		}


		/// <summary>
		/// Releases all resource used by the <see cref="libpsinc.Transport"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.Transport"/>. The
		/// <see cref="Dispose"/> method leaves the <see cref="libpsinc.Transport"/> in an unusable state. After calling
		/// <see cref="Dispose"/>, you must release all references to the <see cref="libpsinc.Transport"/> so the garbage
		/// collector can reclaim the memory that the <see cref="libpsinc.Transport"/> was occupying.</remarks>
		public void Dispose()
		{
			this.Release();
		}


		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="libpsinc.Transport"/> is connected.
		/// </summary>
		/// <value><c>true</c> if connected; otherwise, <c>false</c>.</value>
		public bool Connected 
		{ 
			get 
			{ 
				return this.connected; 
			}
			
			protected set	
			{
				if (this.connected != value)
				{
					this.ConnectionChanged(this.connected = value);
				}
			}
		}

		/// <summary>
		/// Initialise the transport layer with a connection to the 
		/// specified serial and bus.
		/// </summary>
		/// <param name="serial">Serial.</param>
		/// <param name="bus">Bus.</param>
		public bool Initialise(string serial, uint bus)
		{
			this.bus = bus;
			this.serial = serial;

			this.Release();
			bool result = false;

			lock(this)
			{
				IntPtr list;
				int size = Usb.GetDeviceList(IntPtr.Zero, out list);

				if (size > 0)
				{
					var descriptor = new Usb.Descriptor();

					for (int i=0; i<size; i++)
					{
						IntPtr device = Marshal.ReadIntPtr(new IntPtr(list.ToInt64() + i * IntPtr.Size));

						if (device != IntPtr.Zero)
						{
							if (Usb.GetDeviceDescriptor(device, descriptor) == 0)
							{
								if (descriptor.VendorID == VENDOR && descriptor.ProductID == PRODUCT)
								{
									if (this.Claim(device, descriptor.SerialStringIndex, this.serial)) 
									{
										result = true;
										break;
									}
								}
							}
						}
						else break;
					}
				}

				if (list != IntPtr.Zero) Usb.FreeDeviceList(list, 1);
			}
			
			return this.Connected = result;
		}


		/// <summary> Helper function for packing commands into a transmission packet
		/// and performing a transfer </summary>
		/// <returns>False if the command buffer length was invalid </returns>
		/// <param name="command">Buffer containing the command to send (maximum of 5 bytes).</param>
		/// <param name="receive">Buffer to contain the data received (output).</param>
		public bool Command(byte[] command, byte [] receive)
		{
			if (command.Length <= 5)
			{
				byte [] send = { 
					0x00, 0x00, 0x00, 0x00, 0x00,	// Header
					0x00, 0x00, 0x00, 0x00, 0x00, 	// Command copied in here
					0xff 							// Terminator
				};
				
				for (int i=0; i<command.Length; i++) send[i + 5] = command[i];
				
				return this.Transfer(send, receive);
			}
			
			return false;
		}
		

		/// <summary> Helper function for packing commands into a transmission packet
		/// along with a flush command and performing a transfer. </summary>
		/// <returns>False if the command buffer length was invalid </returns>
		/// <param name="command">Buffer containing the command to send (maximum of 5 bytes).</param>
		/// <param name="receive">Buffer to contain the data received (output).</param>
		/// <param name="checkLength">If true the transfer instruction will check the length of the response</param>
		public bool CommandFlush(byte [] command, byte [] receive, bool checkLength = true)
		{
			if (command.Length <= 5)
			{
				byte [] send = { 
					0x00, 0x00, 0x00, 0x00, 0x00,	// Header 
					0x03, 0x00, 0x00, 0x00, 0x00, 	// Flush command
					0x00, 0x00, 0x00, 0x00, 0x00, 	// Command copied in here
					0xff 							// Terminator
				};
				
				for (int i=0; i<command.Length; i++) send[i + 10] = command[i];
				
				return this.Transfer(send, receive, checkLength);
			}
			return false;
		}
		
		/// <summary>
		/// Transfer the data in send[] to the camera and populate receive[] with the 
		/// response data. send[] should contain properly formatted data at this point;
		/// Transfer will not check it.
		/// </summary>
		/// <param name="send">Data to send.</param>
		/// <param name="receive">Data received.</param>
		/// <param name="checkLength">If set to <c>true</c> only report a success if the transfer received
		/// data of length exactly equal to the receive buffer length, otherwise don't care about the 
		/// amount of data received.</param>
		public bool Transfer(byte [] send, byte [] receive, bool checkLength = true)
		{
			bool result		= false;
			bool connection	= true;
			string error	= null;
			
			lock (this)
			{
				this.raiseError	= false;
				result			= this.connected && this.Transfer(send, true, true) && this.Transfer(receive, false, checkLength);
				
				if (this.raiseError) 		
				{
					error		= this.lastError;
					connection	= this.connected;
				}
			}
			
			if (error != null) 	this.TransferError(error);
			if (!connection)	this.ConnectionChanged(false);
			
			return result;
		}

		/// <summary>
		/// Issue a reset command to the camera and return the current
		/// connection status of the camera.
		/// </summary>
		public bool Reset()
		{
			bool result = false;
			
			lock (this)
			{
				if (this.handle != IntPtr.Zero)
				{
					result = Usb.ResetDevice(this.handle) == 0;
					
					if (!result)
					{
						Usb.Close(this.handle);
						
						this.handle = IntPtr.Zero;
					}
				}
			}
			
			return this.Connected = result;
		}


		/// <summary>
		/// Get the serial string of the camera
		/// </summary>
		/// <param name="camera">Camera to query.</param>
		/// <param name="index">Descriptor index to query.</param>
		string Serial(IntPtr camera, int index)
		{	
			Usb.GetStringDescriptorASCII(camera, index, this.buffer, 64);
			return System.Text.Encoding.ASCII.GetString(this.buffer);
		}


		/// <summary>
		/// Claim specified usb device with the specified serial string present
		/// at the descriptor index supplied
		/// </summary>
		/// <param name="device">Device (camera) to claim.</param>
		/// <param name="index">Index of the serial descriptor on the device.</param>
		/// <param name="serial">Serial string</param>
		bool Claim(IntPtr device, int index, string serial)
		{
			uint bus = (uint)Usb.GetBusNumber(device);
			if (this.bus == 0 || this.bus == bus)
			{
			
				if ((serial == string.Empty || Regex.IsMatch(this.Serial(this.handle, index), (serial)))
					&& (Usb.Open(device, ref this.handle) == 0))
				{
					// Disabled this while solving an issue on Windows with 
					// the camera that causes it to get stuck in a strange state.
					//if (Usb.SetConfiguration(this.handle, 1) == 0)
					{
						if (Usb.ClaimInterface(this.handle, 0) == 0)
						{
							return true;
						}
					}

					Usb.Close(this.handle);
					this.handle = IntPtr.Zero;
				}
			}

			return false;
		}

		/// <summary>
		/// Release the currently connected camera.
		/// </summary>
		void Release()
		{
			lock (this)
			{
				if (this.handle != IntPtr.Zero)
				{
					Usb.ReleaseInterface(this.handle, 0);
					Usb.Close(this.handle);

					this.handle = IntPtr.Zero;
				}
			}

			this.Connected = false;
		}


		/// <summary>
		/// Perform a bulk transfer from/to buffer[]
		/// </summary>
		/// <param name="buffer">Buffer of data to be written/written to.</param>
		/// <param name="write">If set to <c>true</c> write the data in buffer[] to the camera.
		/// Otherwise read data from the camera and populate buffer[].</param>
		/// <param name="checkLength">If set to <c>true</c> report a failure on reading if the 
		/// transferred data is not equal to buffer.Length. Otherwise ignore data length 
		/// mismatches on read operations.</param>
		bool Transfer(byte [] buffer, bool write, bool checkLength)
		{
			if (buffer != null)
			{
				if (this.handle != IntPtr.Zero)
				{
					int transferred;
					Usb.Error result = Usb.Error.Other;

					for (int i=0; i<MAX_TRANSFER_ATTEMPTS; i++)
					{
						result = (Usb.Error)Usb.BulkTransfer(this.handle, write ? WRITE_PIPE : READ_PIPE, buffer, buffer.Length, out transferred, TIMEOUT);

						if (result == Usb.Error.Success)
						{
							return (write || checkLength) ? transferred == buffer.Length : true;
						}

						if (result == Usb.Error.NoDevice)
						{
							System.Threading.Thread.Sleep(TIMEOUT / 2);
							break;
						}
					}

					this.raiseError	= true;
					this.connected	= false;
					this.lastError	= result.ToString();
				}

				return false;
			}

			return true;
		}
	}
}


