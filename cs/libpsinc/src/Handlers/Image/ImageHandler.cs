using System;
using System.Drawing;
using System.Drawing.Imaging;


namespace libpsinc
{
	/// <summary>
	/// The abstract base for handling data from an image chip. This should be derived
	/// from to allow decoding of the data to a required image format. Refer to the
	/// DefaultImageHandler to see how the data is decoded to a System.Drawing.Bitmap.
	/// </summary>
	public abstract class ImageHandler
	{
		/// <summary>
		/// Decode a byte array to an image. The source array represents an image
		/// of the specified width and height (note that the image produced may not
		/// be of the same dimensions depending upon the decoding used).
		/// </summary>
		/// <param name="colour">Colour model to use in decoding</param>
		/// <param name="data">Data to decode</param>
		/// <param name="width">Width of the byte array representation of the image</param>
		/// <param name="height">Height of the byte array representation of the image</param>
		public abstract dynamic Decode(ColourMode colour, byte [] data, int width, int height);
	}
}
