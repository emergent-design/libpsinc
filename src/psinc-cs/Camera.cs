using System;
using System.Runtime.InteropServices;


namespace libpsinc
{
	enum ReturnCodes
	{
		Ok					= 0,
		InvalidImage		= -1,
		FileIOError			= -2,
		InvalidCamera		= -3,
		CameraConnected		= -4,
		CameraDisconnected	= -5,
		CameraIOError		= -6,
		UnknownFeature		= -7,
		OutOfRange			= -8
	}


	static class Psinc
	{
		const string LIB = "libpsinc.dll";

		[DllImport(LIB, EntryPoint = "psinc_enable_logging")]		internal static extern void EnableLogging();
		[DllImport(LIB, EntryPoint = "psinc_camera_create")]		internal static extern IntPtr Create();
		[DllImport(LIB, EntryPoint = "psinc_camera_initialise")]	internal static extern int Initialise(IntPtr camera, string serial);
		[DllImport(LIB, EntryPoint = "psinc_camera_delete")]		internal static extern int Delete(IntPtr camera);
		[DllImport(LIB, EntryPoint = "psinc_camera_grab")]			internal static extern int Grab(IntPtr camera, IntPtr image);
		[DllImport(LIB, EntryPoint = "psinc_camera_grab_hdr")]		internal static extern int GrabHDR(IntPtr camera, IntPtr image);
		[DllImport(LIB, EntryPoint = "psinc_camera_set_feature")]	internal static extern int SetFeature(IntPtr camera, string feature, int value);
		[DllImport(LIB, EntryPoint = "psinc_camera_get_feature")]	internal static extern int GetFeature(IntPtr camera, string feature, out int value);
		[DllImport(LIB, EntryPoint = "psinc_camera_set_flash")]		internal static extern int SetFlash(IntPtr camera, byte power);
		[DllImport(LIB, EntryPoint = "psinc_camera_set_context")]	internal static extern int SetContext(IntPtr camera, byte context);
		[DllImport(LIB, EntryPoint = "psinc_camera_connected")]		internal static extern bool Connected(IntPtr camera);
	}
		

	/// <summary>
	/// References an instance of a libpsinc camera in the native library.
	/// </summary>
	public class Camera : IDisposable
	{
		IntPtr camera;


		/// <summary>
		/// Enable logging to the console with "info" level verbosity. Only
		/// necessary to call this once at the start of an application.
		/// </summary>
		public static void EnableLogging()
		{
			Psinc.EnableLogging();
		}


		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.Camera"/> class.
		/// </summary>
		public Camera()
		{
			this.camera = Psinc.Create();
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
			Psinc.Delete(this.camera);
		}


		/// <summary>
		/// Initialise the camera and instruct it to connect to a specific camera.
		/// </summary>
		/// <param name="serial">
		/// An empty serial number will instruct it to connect to the first 
		/// camera it can find. The serial number supports regular expressions and can therefore 
		/// be used to connect to a specific ID regardless of serial number, for example ".*:primary" 
		/// where the ID of the camera has been set to "primary".
		/// </param>
		public void Initialise(string serial)
		{
			Psinc.Initialise(this.camera, serial);
		}


		/// <summary>
		/// Grab a frame and write to the supplied image instance. This function blocks until
		/// an image has been grabbed or an error occurrs.
		/// </summary>
		/// <returns>True if a camera was connected and an image was succesfully grabbed.</returns>
		/// <param name="image">Instance of image in which to write the grabbed frame.</param>
		public bool Grab(Image image)
		{
			return Psinc.Grab(this.camera, image.Pointer) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Grab an HDR frame and write to the supplied image instance. This function blocks until
		/// an image has been grabbed or an error occurrs.
		/// </summary>
		/// <returns>True if a camera was connected and an image was succesfully grabbed.</returns>
		/// <param name="image">Instance of image in which to write the grabbed frame.</param>
		public bool Grab(ImageHDR image)
		{
			return Psinc.GrabHDR(this.camera, image.Pointer) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Retrieve the value for a specific camera feature.
		/// </summary>
		/// <returns>The feature value.</returns>
		/// <param name="feature">Feature name.</param>
		public int GetFeature(string feature)
		{
			int result = 0;

			Psinc.GetFeature(this.camera, feature, out result);

			return result;
		}


		/// <summary>
		/// Set a specific camera feature to the given value.
		/// </summary>
		/// <param name="feature">Feature name.</param>
		/// <param name="value">Value to set</param>
		public bool SetFeature(string feature, int value)
		{
			return Psinc.SetFeature(this.camera, feature, value) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Sets the flash power (0 for disabled).
		/// </summary>
		/// <param name="power">Power level.</param>
		public bool SetFlash(byte power)
		{
			return Psinc.SetFlash(this.camera, power) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Set the camera context (where supported).
		/// </summary>
		/// <param name="context">Context index.</param>
		public bool SetContext(byte context)
		{
			return Psinc.SetContext(this.camera, context) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Check camera connection status
		/// </summary>
		public bool Connected
		{
			get
			{
				return Psinc.Connected(this.camera);
			}
		}


		/// <summary>
		/// Grab a frame from context A and B in rapid succession. Camera is restored to context A when
		/// complete.
		/// </summary>
		/// <returns><c>true</c>, if successful, <c>false</c> otherwise.</returns>
		/// <param name="imageA">Instance of image in which to write the grabbed frame for context A.</param>
		/// <param name="flashA">Required flash power for context A.</param>
		/// <param name="imageB">Instance of image in which to write the grabbed frame for context B.</param>
		/// <param name="flashB">Required flash power for context B.</param>
		public bool GrabMultiple(Image imageA, byte flashA, Image imageB, byte flashB)
		{
			bool result = false;

			Psinc.SetContext(this.camera, 0);
			Psinc.SetFlash(this.camera, flashA);

			if (Psinc.Grab(this.camera, imageA.Pointer) == (int)ReturnCodes.Ok)
			{
				Psinc.SetContext(this.camera, 1);
				Psinc.SetFlash(this.camera, flashB);

				result = Psinc.Grab(this.camera, imageB.Pointer) == (int)ReturnCodes.Ok;

				Psinc.SetContext(this.camera, 0);
			}

			return result;
		}


		/// <summary>
		/// Grab an HDR frame from context A and B in rapid succession.Camera is restored to context A when
		/// complete.
		/// </summary>
		/// <returns><c>true</c>, if successful, <c>false</c> otherwise.</returns>
		/// <param name="imageA">Instance of image in which to write the grabbed frame for context A.</param>
		/// <param name="flashA">Required flash power for context A.</param>
		/// <param name="imageB">Instance of image in which to write the grabbed frame for context B.</param>
		/// <param name="flashB">Required flash power for context B.</param>
		public bool GrabMultiple(ImageHDR imageA, byte flashA, ImageHDR imageB, byte flashB)
		{
			bool result = false;

			Psinc.SetContext(this.camera, 0);
			Psinc.SetFlash(this.camera, flashA);

			if (Psinc.GrabHDR(this.camera, imageA.Pointer) == (int)ReturnCodes.Ok)
			{
				Psinc.SetContext(this.camera, 1);
				Psinc.SetFlash(this.camera, flashB);

				result = Psinc.GrabHDR(this.camera, imageB.Pointer) == (int)ReturnCodes.Ok;

				Psinc.SetContext(this.camera, 0);
			}

			return result;
		}
	}
}
