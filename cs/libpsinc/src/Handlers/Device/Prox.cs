using System;
using System.Threading;

namespace libpsinc.device
{
	/// <summary>
	/// Card presented event.
	/// </summary>
	/// <param name="ID">Unique, immutable identifier of the card</param>
	/// <param name="data">The remainder of the data read</param> 
	public delegate void CardPresentedEvent(uint ID, byte[] data);


	/// <summary>
	/// A simple handler for the Prox reader Device
	/// </summary>
	public class Prox : DeviceHandler, IDisposable
	{

		/// <summary>
		/// Occurs when a proxcard is presented.
		/// </summary>
		public event CardPresentedEvent CardPresented = delegate {};


		/// <summary>
		/// Gets or sets the length of the read.
		/// </summary>
		/// <value>The length of the read, in pages of 16 bytes
		/// (plus a constant 20-byte special read containing the
		/// card ID). The maximum legal value is 15.</value>
		public byte ReadLength 
		{
			get
			{
				return this.readLength;
			}

			set
			{
				if (value >= 0 && value < 16)
				{
					this.readLength = value;
				}
			}
		}

		/// <summary>
		/// Gets or sets amount of time the polling thread
		/// sleeps for each iteration
		/// </summary>
		/// <value>The sleep time (in ms)</value>
		public int PollSleep 			{ get; set; }


		/// <summary>
		/// The name of the device in the Camera device array
		/// </summary>
		protected readonly string name = "Prox";


		/// <summary>
		/// The requested read length.
		/// </summary>
		protected byte readLength = 0;


		/// <summary>
		/// The read length that the Device is currently 
		/// set to (at initialisation this is out-of-bounds
		/// to ensure the desired read length is flushed to
		/// the camera).
		/// </summary>
		protected byte currentLength = 255;


		/// <summary>
		/// The polling thread.
		/// </summary>
		private Thread thread = null;


		/// <summary>
		/// When set, the polling thread will exit
		/// </summary>
		private ManualResetEvent shutdown	= new ManualResetEvent(false);


		/// <summary>
		/// When reset, the polling thread will pause.
		/// </summary>
		private ManualResetEvent pause		= new ManualResetEvent(true);


		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.Prox"/> class.
		/// </summary>
		/// <param name="camera">Camera that the prox reader resides on</param>
		public Prox(Camera camera) : base(camera)
		{
			this.PollSleep 	= 10;
			this.thread		= new Thread(new ThreadStart(this.Entry));
			this.thread.Start();
		}


		/// <summary>
		/// Releases all resource used by the <see cref="libpsinc.Prox"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.Prox"/>. The
		/// <see cref="Dispose"/> method leaves the <see cref="libpsinc.Prox"/> in an unusable state. After calling
		/// <see cref="Dispose"/>, you must release all references to the <see cref="libpsinc.Prox"/> so the garbage
		/// collector can reclaim the memory that the <see cref="libpsinc.Prox"/> was occupying.</remarks>
		public void Dispose()
		{
			this.shutdown.Set();
			this.pause.Set();
			
			if (this.thread!=null)
			{
				if (this.thread.Join(100))
				{
					this.thread.Abort();
				}
				this.thread = null;
			}
		}

		/// <summary>
		/// Resume polling for cards.
		/// </summary>
		public void Resume()
		{
				this.pause.Set();
		}


		/// <summary>
		/// Pause polling for cards.
		/// </summary>
		public void Pause()
		{
			this.pause.Reset();
		}


		/// <summary>
		/// Entry point for the polling thread.
		/// </summary>
		protected void Entry()
		{
			do
			{
				if (this.camera!=null && this.camera.Connected)
				{
					if (this.camera.Devices.ContainsKey(this.name))
					{
						if (this.currentLength!= this.readLength)
						{
							this.currentLength = this.readLength;
							this.camera.Devices[this.name].Initialise(this.currentLength);
						}
						var result = this.camera.Devices[this.name].Read();
						if (result != null && result.Length >= 20)
						{
							byte[] id = new byte[4];
							Array.Copy(result, id, 4);
							this.CardPresented(BitConverter.ToUInt32(id, 0), result);
						}
					}
				}
				this.pause.WaitOne();
			}
			while(!this.shutdown.WaitOne(this.PollSleep));
		}
	}
}
