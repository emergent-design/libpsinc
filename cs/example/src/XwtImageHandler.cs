using System;

using Xwt.Drawing;
using libpsinc;


namespace iconograph
{
	/// <summary>
	/// The default image handler produces a System.Drawing.Bitmap
	/// </summary>
	public class XwtImageHandler : ImageHandler
	{
		public override dynamic Decode(ColourMode colour, byte [] data, int width, int height)
		{
			switch (colour)
			{
				case ColourMode.Monochrome:		return this.DecodeMono(data, width, height);
				case ColourMode.BayerGrey:		return this.DecodeGrey(data, width, height);
				case ColourMode.BayerColour:	return this.DecodeColour(data, width, height);
				default:						return null;
			}
		}
		
		
		/// <summary>
		/// Decode the specified buffer (as data produced by a monochrome imaging chip).
		/// </summary>
		/// <param name="buffer">Buffer to decode</param>
		/// <param name="width">Width of the byte image</param>
		/// <param name="height">Height of the byte image</param>
		unsafe BitmapImage DecodeMono(byte [] buffer, int width, int height)
		{
			BitmapImage result = new ImageBuilder(width, height).ToBitmap(ImageFormat.RGB24);
			
			fixed (byte *b = buffer)
			{
				byte *src = b;

				for (int y=0; y<height; y++)
				{
					for (int x=0; x<width; x++, src++)
					{
						result.SetPixel(x, y, Color.FromBytes(*src, *src, *src));
					}
				}
			}

			return result;
		}
		
		
		/// <summary>
		/// Decodes a bayer image to a greyscale output.
		/// </summary>
		/// <returns>The decoded greyscale image.</returns>
		/// <param name="receive">Raw bayer encoded byte data.</param>
		/// <param name="bayerWidth">Width of the bayer encoded image.</param>
		/// <param name="bayerHeight">Height of the bayer encoded image.</param>
		unsafe BitmapImage DecodeGrey(byte [] receive, int bayerWidth, int bayerHeight)
		{
			if (bayerHeight < 5 || bayerWidth < 5) return null;
			
			int width			= bayerWidth - 4;
			int height			= bayerHeight - 4;
			BitmapImage result	= new ImageBuilder(width, height).ToBitmap(ImageFormat.RGB24);
			
			fixed (byte *src = receive)
			{
				//int jump	= data.Stride - width * 3;
				//byte *dst	= (byte *)data.Scan0.ToPointer();
				
				byte *pg	= src + (bayerWidth << 1) + 2;
				byte *pa	= pg - (bayerWidth << 1);
				byte *pc	= pg - bayerWidth;
				byte *pb	= pc - 1;
				byte *pd	= pc + 1;
				byte *pe	= pg - 2;
				byte *pf	= pg - 1;
				byte *ph	= pg + 1;
				byte *pi	= pg + 2;
				byte *pk	= pg + bayerWidth;
				byte *pj	= pk - 1;
				byte *pl	= pk + 1;
				byte *pm	= pg + (bayerWidth << 1);
				
				int x, y;
				byte value;
				bool oddPixel, oddLine = false;
				
				for (y=0; y<height; y++, pa+=4, pb+=4, pc+=4, pd+=4, pe+=4, pf+=4, pg+=4, ph+=4, pi+=4, pj+=4, pk+=4, pl+=4, pm+=4)
				{
					oddPixel = false;
					for (x=0; x<width; x++)
					{
						if (oddLine)	// GRGR: R or G
						{
							if (oddPixel)	value = Clamp((*pg++ * 36 + ((*pc++ + *pf++ + *ph++ + *pk++ + *pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 5) / 48);
							else			value = Clamp((*pg++ * 36 + ((*pf++ + *ph++ + *pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - *pa++ - *pm++ - *pe++ - *pi++) / 48);
						}
						else			// BGBG: G or B
						{
							if (oddPixel)	value = Clamp((*pg++ * 36 + ((*pf++ + *ph++ + *pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - *pa++ - *pm++ - *pe++ - *pi++) / 48);
							else			value = Clamp((*pg++ * 36 + ((*pc++ + *pf++ + *ph++ + *pk++ + *pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 5) / 48);
						}

						result.SetPixel(x, y, Color.FromBytes(value, value, value));

						oddPixel = !oddPixel;
					}

					oddLine  = !oddLine;
				}
			}
			
			return result;
		}
		
		
		/// <summary>
		/// Decodes a bayer image to a colour output.
		/// </summary>
		/// <returns>The decoded colour image.</returns>
		/// <param name="receive">Raw bayer encoded byte data.</param>
		/// <param name="bayerWidth">Width of the bayer encoded image.</param>
		/// <param name="bayerHeight">Height of the bayer encoded image.</param>
		unsafe BitmapImage DecodeColour(byte [] receive, int bayerWidth, int bayerHeight)
		{
			if (bayerHeight < 5 || bayerWidth < 5) return null;
			
			int width			= bayerWidth - 4;
			int height			= bayerHeight - 4;
			BitmapImage result	= new ImageBuilder(width, height).ToBitmap(ImageFormat.RGB24);
			
			fixed (byte *src = receive)
			{	
				byte *pg	= src + (bayerWidth << 1) + 2;
				byte *pa	= pg - (bayerWidth << 1);
				byte *pc	= pg - bayerWidth;
				byte *pb	= pc - 1;
				byte *pd	= pc + 1;
				byte *pe	= pg - 2;
				byte *pf	= pg - 1;
				byte *ph	= pg + 1;
				byte *pi	= pg + 2;
				byte *pk	= pg + bayerWidth;
				byte *pj	= pk - 1;
				byte *pl	= pk + 1;
				byte *pm	= pg + (bayerWidth << 1);
				
				int x, y;
				byte r, g, b;
				bool oddPixel, oddLine = false;
				
				for (y=0; y<height; y++, pa+=4, pb+=4, pc+=4, pd+=4, pe+=4, pf+=4, pg+=4, ph+=4, pi+=4, pj+=4, pk+=4, pl+=4, pm+=4)
				{
					oddPixel = false;
					for (x=0; x<width; x++)
					{
						if (oddLine)	// GRGR: R or G
						{
							if (oddPixel)
							{
								r = *pg;
								g = Clamp(((*pg << 2) + ((*pc++ + *pf++ + *ph++ + *pk++) << 1) - *pa - *pe - *pi - *pm) >> 3);
								b = Clamp((*pg++ * 12 + ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa++ + *pe++ + *pi++ + *pm++) * 3) >> 4);
							}
							else
							{
								r = Clamp((*pg * 10 + ((*pf++ + *ph++) << 3) - ((*pb + *pd + *pj + *pl + *pe + *pi) << 1) + *pa + *pm) >> 4);
								g = *pg;
								b = Clamp((*pg++ * 10 + ((*pc++ + *pk++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++ + *pa++ + *pm++) << 1) + *pe++ + *pi++) >> 4);
							}
						}
						else			// BGBG: G or B
						{
							if (oddPixel)
							{
								r = Clamp((*pg * 10 + ((*pc++ + *pk++) << 3) - ((*pb + *pd + *pj + *pl + *pa + *pm) << 1) + *pe + *pi) >> 4);
								g = *pg;
								b = Clamp((*pg++ * 10 + ((*pf++ + *ph++) << 3) - ((*pb++ + *pd++ + *pj++ + *pl++ + *pe++ + *pi++) << 1) + *pa++ + *pm++) >> 4);
							}
							else
							{
								r = Clamp((*pg * 12 + ((*pb++ + *pd++ + *pj++ + *pl++) << 2) - (*pa + *pe + *pi + *pm) * 3) >> 4);
								g = Clamp(((*pg << 2) + ((*pc++ + *pf++ + *ph++ + *pk++) << 1) - *pa++ - *pe++ - *pi++ - *pm++) >> 3);
								b = *pg++;
							}
						}

						result.SetPixel(x, y, Color.FromBytes(r, g, b));

						oddPixel = !oddPixel;
					}

					oddLine = !oddLine;
				}
				
			}
			
			return result;
		}
		
		
		/// <summary>
		/// Clamp the specified value to the range 0-255.
		/// </summary>
		/// <returns>The clamped value.</returns>
		/// <param name="value">Value to clamp</param>
		static byte Clamp(int value)
		{
			return (byte)((value > 255) ? 255 : (value < 0) ? 0 : value);
		}
	}
}

