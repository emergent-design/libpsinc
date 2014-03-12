namespace libpsinc.device
{
	/// <summary>
	/// Handler for the LED array to simplify configuring the required LED patterns
	/// </summary>
	public class LEDArray : DeviceHandler
	{
		/// <summary>Overrides the primary mode for specific LEDs and lights them a constant colour</summary>
		public enum Override
		{
			/// <summary>No override</summary>
			None = 0x00,
			
			/// <summary>Override the central LED</summary>
			Central = 0x40,
			
			/// <summary>Override the two outermost LEDs</summary>
			Outer = 0x80,
			
			/// <summary>Override the central and two outermost LEDs</summary>
			CentralAndOuter = 0xc0
		}
		
		
		/// <summary>The primary mode for the LEDs</summary>
		public enum Mode
		{
			/// <summary>Sweep the LEDs back and forth (Cylon style)</summary>
			Sweep = 0x00,
			
			/// <summary>The LEDs will be switched on if the appropriate colour is selected</summary>
			Constant = 0x04,
			
			/// <summary>The LEDs will flash on and off</summary>
			Flash = 0x08,
			
			/// <summary>Just the central LED will be flashed</summary>
			FlashCentre = 0x0c
		}
		
		
		/// <summary>The LED colour</summary>
		public enum Colour
		{
			Off		= 0x00,
			Red		= 0x01,
			Green	= 0x02,
			Blue	= 0x03
		}


		public LEDArray(Camera camera) : base(camera) {}


		/// <summary>The primary mode for the LEDs</summary>
		public Mode PrimaryMode { get; set; }

		/// <summary>The primary LED colour</summary>
		public Colour PrimaryColour { get; set; }

		/// <summary>Overrides the primary mode for specific LEDs and lights them a constant colour</summary>
		public Override OverrideMode { get; set; }

		/// <summary>The override LED colour</summary>
		public Colour OverrideColour { get; set; }
		

		/// <summary>
		/// Write the current configuration to the device. Rather than
		/// writing everytime one of the above properties is changed this
		/// allows you to set them up and then flush the values when ready.
		/// </summary>
		public bool Flush()
		{
			if (this.camera.Devices.ContainsKey("LEDArray"))
			{
				return this.camera.Devices["LEDArray"].Write((byte)(
					(int)this.OverrideMode | ((int)this.OverrideColour << 4) | (int)this.PrimaryMode | (int)this.PrimaryColour
				));
			}

			return false;
		}
	}
}
