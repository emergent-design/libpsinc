#pragma once

#include <stdint.h>

/*
	A wrapper for libpsinc that provides a C API for easier interop with other languages.
*/


extern "C"
{
	struct psinc_camera;	// Structure responsible for communicating with a single camera
	struct emg_image;		// Represents a standard byte image
	struct emg_hdrimage;	// Represents an HDR image (stored as 16-bit)

	enum PSINC_RETURN_CODES
	{
		PSINC_OK					= 0,
		PSINC_INVALID_IMAGE			= -1,
		PSINC_FILE_IO_ERROR			= -2,
		PSINC_INVALID_CAMERA		= -3,
		PSINC_CAMERA_CONNECTED		= -4,
		PSINC_CAMERA_DISCONNECTED	= -5,
		PSINC_CAMERA_IO_ERROR		= -6,
		PSINC_UNKNOWN_FEATURE		= -7,
		PSINC_OUT_OF_RANGE			= -8,

		PSINC_UNKNOWN_DEVICE		= -9,

	};


	// Enable logging to console with "info" level verbosity
	void psinc_enable_logging();


	/* ------------------- Camera ------------------- */

	// Construct an instance of camera
	psinc_camera *psinc_camera_create();

	// Initialise the camera and instruct it to connect to a specific camera.
	// An empty serial number will instruct it to connect to the first camera it can find.
	// The serial number supports regular expressions and can therefore be used to connect
	// to a specific ID regardless of serial number, for example ".*:primary" where the ID
	// of the camera has been set to "primary".
	int psinc_camera_initialise(psinc_camera *camera, const char *serial);

	// Free the memory associated with this camera instance.
	int psinc_camera_delete(psinc_camera *camera);

	// Grab a frame and write to the supplied image instance. This function blocks until
	// an image has been grabbed or an error occurrs.
	int psinc_camera_grab(psinc_camera *camera, emg_image *image);

	// Grab a HDR frame and write to the supplied image instance. This function blocks until
	// an image has been grabbed or an error occurs.
	int psinc_camera_grab_hdr(psinc_camera *camera, emg_hdrimage *image);

	// Set a specific camera feature to the given value.
	int psinc_camera_set_feature(psinc_camera *camera, const char *feature, int value);

	// Retrieve the value for a specific camera feature.
	int psinc_camera_get_feature(psinc_camera *camera, const char *feature, int &value);

	// Set the flash power for this camera.
	int psinc_camera_set_flash(psinc_camera *camera, unsigned char power);

	// Set the camera context (where supported)
	int psinc_camera_set_context(psinc_camera *camera, unsigned char context);

	// Check camera connection status
	bool psinc_camera_connected(psinc_camera *camera);



	/* ------------------- Device ------------------- */

	// Initialise a device by name
	int psinc_device_initialise(psinc_camera *camera, const char *device, unsigned char configuration);

	// Write a byte to a device by name
	int psinc_device_write_byte(psinc_camera *camera, const char *device, unsigned char value);

	// Write a block of data to a device by name
	int psinc_device_write(psinc_camera *camera, const char *device, unsigned char *buffer, int size);

	// Read a block of data from a device by name
	int psinc_device_read(psinc_camera *camera, const char *device, unsigned char *buffer, int size, int &used);


	/* ------------------- Image ------------------- */

	// Allocate memory for an image instance.
	emg_image *emg_image_create(bool colour);

	// Free the memory associated with this image instance.
	int emg_image_delete(emg_image *image);

	// Save image to the given path (filetype is based upon extension used).
	// If raw is set to true then the filetype is ignored and the data saved out in a custom raw format.
	int emg_image_save(emg_image *image, const char *path, bool raw);

	// Load an image from the given path. If raw is set to true then the file data is assumed
	// to be of a custom raw format.
	int emg_image_load(emg_image *image, const char *path, bool raw);

	// Retrieve the image dimensions.
	int emg_image_properties(emg_image *image, int &width, int &height, int &depth);

	// Copy the raw image data into the supplied data buffer. The buffer must be allocated
	// to be at least stride * height in size.
	// Stride is the number of bytes allocated for a single row of pixels in the data buffer.
	// Stride is usually calculated as: bytes-per-channel x image-depth x image-width + padding.
	// The bgr flag indicates whether or not the data is expected in a BGR byte-order (which is
	// common on little-endian machines).
	int emg_image_get(emg_image *image, unsigned char *data, int stride, bool bgr);

	// Set the data in the image. The image is resized to match the provided dimensions.
	// Stride indicates the number of bytes for a single row of pixels in the source data.
	// The bgr flag indicates whether or not the source data is in a BGR byte-order.
	int emg_image_set(emg_image *image, unsigned char *data, int width, int height, int depth, int stride, bool bgr);

	// Retrieve a reference to the start of the pixel data. If this is a colour image then the
	// internal channel order is RGB. This reference is invalid if the image is deleted or
	// resized.
	unsigned char *emg_image_data(emg_image *image);



	/* ------------------- HDR Image ------------------- */

	// Allocate memory for an image instance.
	emg_hdrimage *emg_hdrimage_create(bool colour);

	// Free the memory associated with this image instance.
	int emg_hdrimage_delete(emg_hdrimage *image);

	// Save image to the given path (filetype is based upon extension used). Some filetypes
	// may not support 16-bit images. If raw is set to true then the filetype is ignored and
	// the data saved out in a custom raw format.
	int emg_hdrimage_save(emg_hdrimage *image, const char *path, bool raw);

	// Load an image from the given path. If raw is set to true then the file data is assumed
	// to be of a custom raw format.
	int emg_hdrimage_load(emg_hdrimage *image, const char *path, bool raw);

	// Retrieve the image dimensions.
	int emg_hdrimage_properties(emg_hdrimage *image, int &width, int &height, int &depth);

	// Copy the raw image data into the supplied data buffer. The buffer must be allocated
	// to be at least stride * height in size.
	// Stride is the number of bytes allocated for a single row of pixels in the data buffer.
	// Stride is usually calculated as: bytes-per-channel x image-depth x image-width + padding.
	// The bgr flag indicates whether the data is expected in a BGR byte-order (which is common
	// on little-endian machines).
	int emg_hdrimage_get(emg_hdrimage *image, unsigned char *data, int stride, bool bgr);

	// Set the data in the image. The image is resized to match the provided dimensions.
	// Stride indicates the number of bytes for a single row of pixels in the source data.
	// The bgr flag indicates whether or not the source data is in a BGR byte-order.
	int emg_hdrimage_set(emg_hdrimage *image, unsigned char *data, int width, int height, int depth, int stride, bool bgr);

	// Retrieve a reference to the start of the pixel data. If this is a colour image then the
	// internal channel order is RGB. This reference is invalid if the image is deleted or
	// resized.
	uint16_t *emg_hdrimage_data(emg_hdrimage *image);
}
