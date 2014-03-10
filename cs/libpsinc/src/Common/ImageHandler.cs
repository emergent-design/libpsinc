using System;
using System.Drawing;
using System.Drawing.Imaging;


namespace libpsinc
{
	/// <summary>
	/// Image handler. Converts an incoming byte array from the camera into a Bitmap
	/// </summary>
	internal class ImageHandler
	{
		/// <summary>
		/// Decode a byte array to in image. The source array represents an image
		/// of the specified width and height (note that the image produced may not
		/// be of the same dimensions depending upon the decoding used).
		/// </summary>
		/// <param name="colour">Colour model to use in decoding</param>
		/// <param name="data">Data to decode</param>
		/// <param name="width">Width of the byte array representation of the image</param>
		/// <param name="height">Height of the byte array representation of the image</param>
		public static Bitmap Decode(ColourMode colour, byte [] data, int width, int height)
		{
			switch (colour)
			{
				case ColourMode.Monochrome:		return Decode(data, width, height);
				case ColourMode.BayerGrey:		return Bayer.DecodeGrey(data, width, height);
				case ColourMode.BayerColour:	return Bayer.DecodeColour(data, width, height);
				default:						return null;
			}
		}

		/// <summary>
		/// Decode the specified buffer (as data produced by a monochrome imaging chip).
		/// </summary>
		/// <param name="buffer">Buffer to decode</param>
		/// <param name="width">Width of the byte image</param>
		/// <param name="height">Height of the byte image</param>
		unsafe static Bitmap Decode(byte [] buffer, int width, int height)
		{
			//Bitmap result = new Bitmap(width, height, PixelFormat.Format24bppRgb);
			Bitmap result	= new Bitmap(width, height, PixelFormat.Format24bppRgb);
			BitmapData data	= result.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);
			
			fixed (byte *b = buffer)
			{
				int size	= width * height;
				int jump	= data.Stride - width * 3;
				byte *dst	= (byte *)data.Scan0.ToPointer();
				byte *src	= b;

				if (jump > 0)
				{
					for (int y=0; y<height; y++, dst += jump)
					{
						for (int x=0; x<width; x++)
						{
							*dst++ = *src;
							*dst++ = *src;
							*dst++ = *src++;
						}
					}
				}
				else for (int i=0; i<size; i++)
				{
					*dst++ = *src;
					*dst++ = *src;
					*dst++ = *src++;
				}
			}
			
			result.UnlockBits(data);
			
			return result;
		}
	}
}