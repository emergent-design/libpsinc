namespace libpsinc.device
{
	/// <summary>
	/// Helper for the electronic lock device
	/// </summary>
	public class SimpleLock: DeviceHandler
	{
		public SimpleLock(Camera camera) : base(camera) {}


		/// <summary>
		/// Lock the electronic lock
		/// </summary>
		/// <returns>True if successful and false if the camera is not connected or has no lock device</returns>
		public bool Lock()
		{
			if (this.camera.Devices.ContainsKey("Lock"))
			{
				return this.camera.Devices["Lock"].Write(0x01);
			}

			return false;
		}


		/// <summary>
		/// Unlock the electronic lock
		/// </summary>
		/// <returns>True if successful and false if the camera is not connected or has no lock device</returns>
		public bool Unlock()
		{
			if (this.camera.Devices.ContainsKey("Lock"))
			{
				return this.camera.Devices["Lock"].Write(0x00);
			}

			return false;
		}
	}
}