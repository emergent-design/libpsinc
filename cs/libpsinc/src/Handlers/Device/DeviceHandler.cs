namespace libpsinc.device
{
	/// <summary>
	/// The abstract base for dealing with devices
	/// </summary>
	public abstract class DeviceHandler
	{
		protected Camera camera;


		public DeviceHandler(Camera camera)
		{
			this.camera = camera;
		}
	}
}
