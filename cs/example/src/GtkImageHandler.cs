using System;

using Gdk;
using libpsinc;


namespace iconograph
{
	/// <summary>
	/// An image handler for Gtk applications that produces a Pixbuf image
	/// </summary>
	public class GtkImageHandler : ImageHandler
	{
		public override object Decode(ColourMode colour, byte [] buffer, int width, int height)
		{
			Pixbuf result	= null;
			int w 			= colour == ColourMode.Monochrome ? width : width - 4;
			int h			= colour == ColourMode.Monochrome ? height : height - 4;
			
			if (w > 0 && h > 0)
			{
				result = new Pixbuf(Colorspace.Rgb, false, 8, w, h);
				
				unsafe 
				{
					switch (colour)
					{
						case ColourMode.Monochrome:		this.DecodeMono(buffer, (byte *)result.Pixels, width, height, result.Rowstride);			break;
						case ColourMode.BayerGrey:		this.DecodeGrey(buffer, (byte *)result.Pixels, width, height, result.Rowstride);			break;
						case ColourMode.BayerColour:	this.DecodeColour(buffer, (byte *)result.Pixels, width, height, result.Rowstride, true);	break;
					}
				}
			}
			
			return result;
		}
	}
}
