#include "psinc.h"
#include <emergent/image/Image.hpp>

using namespace emg;


template <typename T> int _emg_delete(ImageBase<T> *image)
{
	if (!image) return PSINC_INVALID_IMAGE;

	delete image;
	return PSINC_OK;
}


template <typename T> int _emg_save(ImageBase<T> *image, const char *path, bool raw)
{
	if (!image) return PSINC_INVALID_IMAGE;
	if (raw) 	return image->SaveRaw(path) ? PSINC_OK : PSINC_FILE_IO_ERROR;
	else		return image->Save(path) ? PSINC_OK : PSINC_FILE_IO_ERROR;
}


template <typename T> int _emg_load(ImageBase<T> *image, const char *path, bool raw)
{
	if (!image) return PSINC_INVALID_IMAGE;
	if (raw) 	return image->LoadRaw(path) ? PSINC_OK : PSINC_FILE_IO_ERROR;
	else		return image->Load(path) ? PSINC_OK : PSINC_FILE_IO_ERROR;
}


template <typename T> int _emg_properties(ImageBase<T> *image, int &width, int &height, int &depth)
{
	if (!image) return PSINC_INVALID_IMAGE;

	width	= image->Width();
	height	= image->Height();
	depth	= image->Depth();

	return PSINC_OK;
}


template <typename T> void _emg_bgr(T *src, T *dst, int width, int height, int stride)
{
	int x, y;
	int jump = (stride / sizeof(T)) - width * 3;

	for (y=0; y<height; y++, dst += jump)
	{
		for (x=0; x<width; x++, src+=3)
		{
			*dst++ = src[2];
			*dst++ = src[1];
			*dst++ = src[0];
		}
	}
}


template <typename T> int _emg_get(ImageBase<T> *image, byte *data, int stride, bool bgr)
{
	if (!image) return PSINC_INVALID_IMAGE;

	int width	= image->Width();
	int height	= image->Height();
	int depth	= image->Depth();
	int row 	= width * depth;

	if (depth == 3 && bgr)
	{
		_emg_bgr(image->Data(), (T *)data, width, height, stride);
	}
	else if (stride == row * sizeof(T))
	{
		memcpy(data, image->Data(), width * height * depth * sizeof(T));
	}
	else
	{
		T *src = image->Data();

		for (int y=0; y<height; y++, data += stride, src += row)
		{
			memcpy(data, src, row * sizeof(T));
		}
	}

	return PSINC_OK;
}


template <typename T> int _emg_set(ImageBase<T> *image, byte *data, int width, int height, int depth, int stride, bool bgr)
{
	if (!image) return PSINC_INVALID_IMAGE;

	int row = width * depth;
	image->Resize(width, height, depth);

	if (depth == 3 && bgr)
	{
		_emg_bgr((T *)data, image->Data(), width, height, stride);
	}
	else if (stride == row * sizeof(T))
	{
		memcpy(image->Data(), data, width * height * depth * sizeof(T));
	}
	else
	{
		T *dst = image->Data();

		for (int y=0; y<height; y++, data += stride, dst += row)
		{
			memcpy(dst, data, row * sizeof(T));
		}
	}

	return PSINC_OK;
}



extern "C"
{
	emg_image *emg_image_create(bool colour)																				{ return reinterpret_cast<emg_image *>(new ImageBase<byte>(colour ? 3 : 1)); }
	emg_hdrimage *emg_hdrimage_create(bool colour)																			{ return reinterpret_cast<emg_hdrimage *>(new ImageBase<uint16_t>(colour ? 3 : 1));	}
	int emg_image_delete(emg_image *image)																					{ return _emg_delete(reinterpret_cast<ImageBase<byte> *>(image)); }
	int emg_hdrimage_delete(emg_hdrimage *image)																			{ return _emg_delete(reinterpret_cast<ImageBase<uint16_t> *>(image)); }
	int emg_image_save(emg_image *image, const char *path, bool raw) 														{ return _emg_save(reinterpret_cast<ImageBase<byte> *>(image), path, raw); }
	int emg_hdrimage_save(emg_hdrimage *image, const char *path, bool raw)													{ return _emg_save(reinterpret_cast<ImageBase<uint16_t> *>(image), path, raw); }
	int emg_image_load(emg_image *image, const char *path, bool raw)														{ return _emg_load(reinterpret_cast<ImageBase<byte> *>(image), path, raw); }
	int emg_hdrimage_load(emg_hdrimage *image, const char *path, bool raw)													{ return _emg_load(reinterpret_cast<ImageBase<uint16_t> *>(image), path, raw); }
	int emg_image_properties(emg_image *image, int &width, int &height, int &depth)											{ return _emg_properties(reinterpret_cast<ImageBase<byte> *>(image), width, height, depth); }
	int emg_hdrimage_properties(emg_hdrimage *image, int &width, int &height, int &depth)									{ return _emg_properties(reinterpret_cast<ImageBase<uint16_t> *>(image), width, height, depth); }
	int emg_image_get(emg_image *image, unsigned char *data, int stride, bool bgr)											{ return _emg_get(reinterpret_cast<ImageBase<byte> *>(image), data, stride, bgr); }
	int emg_hdrimage_get(emg_hdrimage *image, unsigned char *data, int stride, bool bgr)									{ return _emg_get(reinterpret_cast<ImageBase<uint16_t> *>(image), data, stride, bgr); }
	int emg_image_set(emg_image *image, unsigned char *data, int width, int height, int depth, int stride, bool bgr)		{ return _emg_set(reinterpret_cast<ImageBase<byte> *>(image), data, width, height, depth, stride, bgr); }
	int emg_hdrimage_set(emg_hdrimage *image, unsigned char *data, int width, int height, int depth, int stride, bool bgr)	{ return _emg_set(reinterpret_cast<ImageBase<uint16_t> *>(image), data, width, height, depth, stride, bgr); }
}
