using System;


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
		/// <param name="buffer">Data to decode</param>
		/// <param name="width">Width of the byte array representation of the image</param>
		/// <param name="height">Height of the byte array representation of the image</param>
		public abstract object Decode(ColourMode colour, byte [] buffer, int width, int height);

			
		/// <summary>
		/// Decode the specified buffer (as data produced by a monochrome imaging chip).
		/// </summary>
		/// <param name="buffer">Raw bayer encoded byte data.</param>]
		/// <param name="dst">Destination pointer to the raw pixels of a 24-bit image</param>
		/// <param name="width">Width of the byte image</param>
		/// <param name="height">Height of the byte image</param>
		/// <param name="stride">Number of bytes in a row of the destination image</param>
		unsafe protected void DecodeMono(byte [] buffer, byte *dst, int width, int height, int stride)
		{	
			fixed (byte *b = buffer)
			{
				int size	= width * height;
				int jump	= stride - width * 3;
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
		}
		
		
		/// <summary>
		/// Decodes a bayer image to a greyscale output.
		/// </summary>
		/// <returns>The decoded greyscale image.</returns>
		/// <param name="buffer">Raw bayer encoded byte data.</param>]
		/// <param name="dst">Destination pointer to the raw pixels of a 24-bit image</param>
		/// <param name="bayerWidth">Width of the bayer encoded image.</param>
		/// <param name="bayerHeight">Height of the bayer encoded image.</param>
		/// <param name="stride">Number of bytes in a row of the destination image</param>
		unsafe protected void DecodeGrey(byte [] buffer, byte *dst, int bayerWidth, int bayerHeight, int stride)
		{
			if (bayerHeight < 5 || bayerWidth < 5) return;
			
			int width	= bayerWidth - 4;
			int height	= bayerHeight - 4;
			int jump	= stride - width * 3;
			
			fixed (byte *src = buffer)
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
						
						*dst++		= value;
						*dst++		= value;
						*dst++		= value;
						oddPixel	= !oddPixel;
					}
					
					dst		+= jump;
					oddLine  = !oddLine;
				}
			}
		}
		
		
		/// <summary>
		/// Decodes a bayer image to a colour output.
		/// </summary>
		/// <returns>The decoded colour image.</returns>
		/// <param name="buffer">Raw bayer encoded byte data.</param>]
		/// <param name="dst">Destination pointer to the raw pixels of a 24-bit image</param>
		/// <param name="bayerWidth">Width of the bayer encoded image.</param>
		/// <param name="bayerHeight">Height of the bayer encoded image.</param>
		/// <param name="stride">Number of bytes in a row of the destination image</param>
		/// <param name="rgb">Flag to indicate channel order. If true the order is RGB otherwise it is BGR</param>
		unsafe protected void DecodeColour(byte [] buffer, byte *dst, int bayerWidth, int bayerHeight, int stride, bool rgb)
		{
			if (bayerHeight < 5 || bayerWidth < 5) return;
			
			int width	= bayerWidth - 4;
			int height	= bayerHeight - 4;
			int jump	= stride - width * 3;
			
			fixed (byte *src = buffer)
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
						
						if (rgb)
						{
							*dst++ = r; *dst++ = g; *dst++ = b;
						}
						else
						{
							*dst++ = b; *dst++ = g; *dst++ = r;
						}

						oddPixel = !oddPixel;
					}
					
					dst		+= jump;
					oddLine  = !oddLine;
				}	
			}
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
