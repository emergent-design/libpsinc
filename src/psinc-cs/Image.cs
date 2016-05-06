using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;


namespace libpsinc
{
	static class Emg
	{
		const string LIB = "libpsinc.dll";

		[DllImport(LIB, EntryPoint = "emg_image_create")]			internal static extern IntPtr Create(bool colour);
		[DllImport(LIB, EntryPoint = "emg_image_delete")]			internal static extern int Delete(IntPtr image);
		[DllImport(LIB, EntryPoint = "emg_image_save")]				internal static extern int Save(IntPtr image, string path, bool raw);
		[DllImport(LIB, EntryPoint = "emg_image_load")]				internal static extern int Load(IntPtr image, string path, bool raw);
		[DllImport(LIB, EntryPoint = "emg_image_properties")]		internal static extern int Properties(IntPtr image, out int width, out int height, out int depth);
		[DllImport(LIB, EntryPoint = "emg_image_get")]				internal static extern int Get(IntPtr image, IntPtr data, int stride, bool bgr);
		[DllImport(LIB, EntryPoint = "emg_image_set")]				internal static extern int Set(IntPtr image, IntPtr data, int width, int height, int depth, int stride, bool bgr);

		[DllImport(LIB, EntryPoint = "emg_hdrimage_create")]		internal static extern IntPtr CreateHDR(bool colour);
		[DllImport(LIB, EntryPoint = "emg_hdrimage_delete")]		internal static extern int DeleteHDR(IntPtr image);
		[DllImport(LIB, EntryPoint = "emg_hdrimage_save")]			internal static extern int SaveHDR(IntPtr image, string path, bool raw);
		[DllImport(LIB, EntryPoint = "emg_hdrimage_load")]			internal static extern int LoadHDR(IntPtr image, string path, bool raw);
		[DllImport(LIB, EntryPoint = "emg_hdrimage_properties")]	internal static extern int PropertiesHDR(IntPtr image, out int width, out int height, out int depth);
		[DllImport(LIB, EntryPoint = "emg_hdrimage_get")]			internal static extern int GetHDR(IntPtr image, IntPtr data, int stride, bool bgr);
		[DllImport(LIB, EntryPoint = "emg_hdrimage_set")]			internal static extern int SetHDR(IntPtr image, IntPtr data, int width, int height, int depth, int stride, bool bgr);
	}


	/// <summary>
	/// Simple structure used for retrieving image properties.
	/// </summary>
	public class ImageProperties
	{
		public int width	= 0;
		public int height	= 0;
		public int depth	= 0;
	}


	/// <summary>
	/// References an instance of a libemergent image in the native library.
	/// </summary>
	public class Image : IDisposable
	{
		/// <summary>
		/// Gets the pointer to the underlying unmanaged image instance.
		/// </summary>
		public IntPtr Pointer { get; private set; }


		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.Image"/> class.
		/// </summary>
		/// <param name="colour">
		/// If set to <c>true</c> a 24-bit colour image will be allocated. 
		/// If <c>false</c> an 8-bit greyscale image will be allocated.
		/// </param>
		public Image(bool colour = false)
		{
			this.Pointer = Emg.Create(colour);
		}


		/// <summary>
		/// Releases all resource used by the <see cref="libpsinc.Image"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.Image"/>. The <see cref="Dispose"/>
		/// method leaves the <see cref="libpsinc.Image"/> in an unusable state. After calling <see cref="Dispose"/>, you must
		/// release all references to the <see cref="libpsinc.Image"/> so the garbage collector can reclaim the memory that
		/// the <see cref="libpsinc.Image"/> was occupying.</remarks>
		public void Dispose()
		{
			Emg.Delete(this.Pointer);
		}


		/// <summary>
		/// Save image to the given path (filetype is based upon extension used).
		/// If raw is set to true then the filetype is ignored and the data saved out in a custom raw format.
		/// </summary>
		public bool Save(string path, bool raw = false)
		{
			return Emg.Save(this.Pointer, path, raw) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Load an image from the given path.
		/// If raw is set to true then the file data is assumed to be of a custom raw format.
		/// </summary>
		public bool Load(string path, bool raw = false)
		{
			return Emg.Load(this.Pointer, path, raw) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Retrieve the image dimensions.
		/// </summary>
		public ImageProperties Properties()
		{
			var result = new ImageProperties();

			Emg.Properties(this.Pointer, out result.width, out result.height, out result.depth);

			return result;
		}


		/// <summary>
		/// Retrieve the image from unmanaged memory into a Bitmap instance.
		/// </summary>
		public Bitmap Get()
		{
			var properties	= this.Properties();
			var result		= new Bitmap(properties.width, properties.height, properties.depth == 3 ? PixelFormat.Format24bppRgb : PixelFormat.Format8bppIndexed);
			var data		= result.LockBits(new Rectangle(0, 0, properties.width, properties.height), ImageLockMode.ReadWrite, result.PixelFormat);

			Emg.Get(this.Pointer, data.Scan0, data.Stride, true);

			result.UnlockBits(data);

			if (properties.depth == 1)
			{
				var palette = result.Palette;

				for (int i=0; i<256; i++) 
				{
					palette.Entries[i] = Color.FromArgb(i, i, i);
				}

				result.Palette = palette;
			}

			return result;
		}


		/// <summary>
		/// Set the image in unmanaged memory from the supplied Bitmap.
		/// </summary>
		public bool Set(Bitmap image)
		{
			if (image.PixelFormat == PixelFormat.Format24bppRgb || image.PixelFormat == PixelFormat.Format8bppIndexed)
			{
				var data = image.LockBits(new Rectangle(0, 0, image.Width, image.Height), ImageLockMode.ReadWrite, image.PixelFormat);

				Emg.Set(this.Pointer, data.Scan0, image.Width, image.Height, image.PixelFormat == PixelFormat.Format24bppRgb ? 3 : 1, data.Stride, true);

				image.UnlockBits(data);

				return true;
			}

			return false;
		}
	}


	/// <summary>
	/// References an instance of a libemergent HDR image in the native library.
	/// </summary>
	public class ImageHDR : IDisposable
	{
		/// <summary>
		/// Gets the pointer to the underlying unmanaged image instance.
		/// </summary>
		public IntPtr Pointer { get; private set; }


		/// <summary>
		/// Initializes a new instance of the <see cref="libpsinc.ImageHDR"/> class.
		/// </summary>
		/// <param name="colour">
		/// If set to <c>true</c> a 48-bit colour image will be allocated. 
		/// If <c>false</c> a 16-bit greyscale image will be allocated.
		/// </param>
		public ImageHDR(bool colour = false)
		{
			this.Pointer = Emg.CreateHDR(colour);
		}


		/// <summary>
		/// Releases all resource used by the <see cref="libpsinc.ImageHDR"/> object.
		/// </summary>
		/// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="libpsinc.ImageHDR"/>. The
		/// <see cref="Dispose"/> method leaves the <see cref="libpsinc.ImageHDR"/> in an unusable state. After calling
		/// <see cref="Dispose"/>, you must release all references to the <see cref="libpsinc.ImageHDR"/> so the garbage
		/// collector can reclaim the memory that the <see cref="libpsinc.ImageHDR"/> was occupying.</remarks>
		public void Dispose()
		{
			Emg.DeleteHDR(this.Pointer);
		}


		/// <summary>
		/// Save image to the given path (filetype is based upon extension used).
		/// If raw is set to true then the filetype is ignored and the data saved out in a custom raw format.
		/// </summary>
		public bool Save(string path, bool raw = false)
		{
			return Emg.SaveHDR(this.Pointer, path, raw) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Load an image from the given path.
		/// If raw is set to true then the file data is assumed to be of a custom raw format.
		/// </summary>
		public bool Load(string path, bool raw = false)
		{
			return Emg.LoadHDR(this.Pointer, path, raw) == (int)ReturnCodes.Ok;
		}


		/// <summary>
		/// Retrieve the image dimensions.
		/// </summary>
		public ImageProperties Properties()
		{
			var result = new ImageProperties();

			Emg.PropertiesHDR(this.Pointer, out result.width, out result.height, out result.depth);

			return result;
		}


		/// <summary>
		/// Retrieve the image from unmanaged memory into a Bitmap instance.
		/// </summary>
		public Bitmap Get()
		{
			var properties	= this.Properties();
			var result		= new Bitmap(properties.width, properties.height, properties.depth == 3 ? PixelFormat.Format48bppRgb : PixelFormat.Format16bppGrayScale);
			var data		= result.LockBits(new Rectangle(0, 0, properties.width, properties.height), ImageLockMode.ReadWrite, result.PixelFormat);

			Emg.GetHDR(this.Pointer, data.Scan0, data.Stride, true);

			result.UnlockBits(data);

			return result;
		}


		/// <summary>
		/// Set the image in unmanaged memory from the supplied Bitmap.
		/// </summary>
		public bool Set(Bitmap image)
		{
			if (image.PixelFormat == PixelFormat.Format48bppRgb || image.PixelFormat == PixelFormat.Format16bppGrayScale)
			{
				var data = image.LockBits(new Rectangle(0, 0, image.Width, image.Height), ImageLockMode.ReadWrite, image.PixelFormat);

				Emg.SetHDR(this.Pointer, data.Scan0, image.Width, image.Height, image.PixelFormat == PixelFormat.Format48bppRgb ? 3 : 1, data.Stride, true);

				image.UnlockBits(data);

				return true;
			}

			return false;
		}
	}
}

