#include "psinc.h"
#include "psinc/Camera.h"

using namespace std;
using namespace psinc;
using namespace emg;


extern "C"
{

	int psinc_device_initialise(psinc_camera *camera, const char *device, unsigned char configuration)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		auto c = reinterpret_cast<Camera *>(camera);

		return c->devices.count(device) == 0
			? PSINC_UNKNOWN_DEVICE
			: c->devices[device].Initialise(configuration) ? PSINC_OK : PSINC_CAMERA_IO_ERROR;
	}


	int psinc_device_write_byte(psinc_camera *camera, const char *device, unsigned char value)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		auto c = reinterpret_cast<Camera *>(camera);

		return c->devices.count(device) == 0
			? PSINC_UNKNOWN_DEVICE
			: c->devices[device].Write(value) ? PSINC_OK : PSINC_CAMERA_IO_ERROR;
	}


	int psinc_device_write(psinc_camera *camera, const char *device, unsigned char *buffer, int size)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		auto c = reinterpret_cast<Camera *>(camera);

		return c->devices.count(device) == 0
		 	? PSINC_UNKNOWN_DEVICE
		 	: c->devices[device].Write(buffer, size) ? PSINC_OK : PSINC_CAMERA_IO_ERROR;
	}


	int psinc_device_read(psinc_camera *camera, const char *device, unsigned char *buffer, int size, int &used)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		auto c = reinterpret_cast<Camera *>(camera);

		// Perhaps we need some sort of buffer view to handle this case without
		// unnecessary memory allocation and copying.
		Buffer<byte> data(size);

		if (c->devices.count(device))
		{
			if (c->devices[device].Read(data))
			{
				used = data.Size();
				memcpy(buffer, data.Data(), used);
				return PSINC_OK;
			}
			else return PSINC_CAMERA_IO_ERROR;
		}

		return PSINC_UNKNOWN_DEVICE;
	}

}
