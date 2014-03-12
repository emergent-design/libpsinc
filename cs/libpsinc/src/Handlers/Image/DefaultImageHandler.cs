using System;
using System.Drawing;
using System.Drawing.Imaging;


namespace libpsinc
{
	/// <summary>
	/// The default image handler produces a System.Drawing.Bitmap
	/// </summary>
	public class DefaultImageHandler : ImageHandler
	{
		public override object Decode(ColourMode colour, byte [] buffer, int width, int height)
		{
			Bitmap result	= null;
			int w 			= colour == ColourMode.Monochrome ? width : width - 4;
			int h			= colour == ColourMode.Monochrome ? height : height - 4;

			if (w > 0 && h > 0)
			{
				result		= new Bitmap(w, h, PixelFormat.Format24bppRgb);
				var data	= result.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);

				unsafe 
				{
					switch (colour)
					{
						case ColourMode.Monochrome:		this.DecodeMono(buffer, (byte *)data.Scan0.ToPointer(), width, height, data.Stride);			break;
						case ColourMode.BayerGrey:		this.DecodeGrey(buffer, (byte *)data.Scan0.ToPointer(), width, height, data.Stride);			break;
						case ColourMode.BayerColour:	this.DecodeColour(buffer, (byte *)data.Scan0.ToPointer(), width, height, data.Stride, false);	break;
					}
				}

				result.UnlockBits(data);
			}

			return result;
		}
	}
}

