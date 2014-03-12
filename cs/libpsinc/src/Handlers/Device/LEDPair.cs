namespace libpsinc.device
{

	/// <summary>
	/// A simple handler for the LED pair Device
	/// </summary>
	class LEDPair : DeviceHandler
	{
		///<summary>
		///The colours available for each LED. 
		///</summary>
		public enum Colour
		{
			Off		= 0,
			Red		= 1,
			Green 	= 2
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.LEDPair"/> class.
		/// </summary>
		/// <param name="camera">Camera that the LED pair resides on</param>
		public LEDPair(Camera camera) : base(camera) {}


		/// <summary>
		/// Flush the specified colours to the LED pair
		/// </summary>
		/// <param name="a">The colour for LED a</param>
		/// <param name="b">The colour for LED b</param>
		/// <returns>True if the device was set, otherwise false.</returns>
		public bool Flush( Colour a, Colour b )
		{
			if (this.camera!= null && this.camera.Devices.ContainsKey("LEDPair"))
			{
				return this.camera.Devices["LEDPair"].Write((byte)(((byte)a << 2) | (byte)b));
			}
			return false;
		}
	}
}
