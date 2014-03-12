using System;


namespace libpsinc.device
{
	/// <summary>
	/// Helper to read and format the serial number of the camera
	/// </summary>
	public class SerialNumber : DeviceHandler
	{
		public SerialNumber(Camera camera) : base(camera) {}


		/// <summary>
		/// Read the serial number
		/// </summary>
		/// <returns>A hex string or null if a camera is not connected or has no serial device</returns>
		public string Read()
		{
			if (this.camera.Devices.ContainsKey("Serial"))
			{
				var data = this.camera.Devices["Serial"].Read();

				return data != null ? String.Concat(Array.ConvertAll(data, x => x.ToString("x2"))) : null;
			}

			return null;
		}
	}
}
