#include "psinc.h"
#include "psinc/Camera.h"
#include "psinc/handlers/ImageHandler.hpp"
#include <emergent/logger/Logger.hpp>
#include <thread>

using namespace std;
using namespace psinc;
using namespace emg;


template <typename T> int _psinc_grab(Camera *camera, ImageBase<T> *image)
{
	if (!camera)	return PSINC_INVALID_CAMERA;
	if (!image)		return PSINC_INVALID_IMAGE;

	int result = PSINC_OK;
	ImageHandler<T> handler(*image);

	camera->GrabImage(Camera::Mode::Normal, handler, [&](bool status) {
		result = status ? PSINC_OK : PSINC_CAMERA_IO_ERROR;
		return false;
	});

	while (camera->Grabbing())
	{
		this_thread::sleep_for(1ms);
	}

	return result;
}


extern "C"
{
	void psinc_enable_logging()
	{
		Log::Initialise({ unique_ptr<logger::Sink>(new logger::Console()) });
		Log::Verbosity(Severity::Info);
	}


	psinc_camera *psinc_camera_create()
	{
		return reinterpret_cast<psinc_camera *>(new Camera());
	}


	int psinc_camera_initialise(psinc_camera *camera, const char *serial)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		reinterpret_cast<Camera *>(camera)->Initialise(serial);

		return PSINC_OK;
	}


	int psinc_camera_delete(psinc_camera *camera)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		delete reinterpret_cast<Camera *>(camera);

		return PSINC_OK;
	}


	int psinc_camera_grab(psinc_camera *camera, emg_image *image)
	{
		return _psinc_grab(reinterpret_cast<Camera *>(camera), reinterpret_cast<ImageBase<byte> *>(image));
	}


	int psinc_camera_grab_hdr(psinc_camera *camera, emg_hdrimage *image)
	{
		return _psinc_grab(reinterpret_cast<Camera *>(camera), reinterpret_cast<ImageBase<uint16_t> *>(image));
	}


	int psinc_camera_set_feature(psinc_camera *camera, const char *feature, int value)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		auto c = reinterpret_cast<Camera *>(camera);

		return c->features.count(feature) == 0 ? PSINC_UNKNOWN_FEATURE : c->features[feature].Set(value) ? PSINC_OK : PSINC_OUT_OF_RANGE;
	}


	int psinc_camera_get_feature(psinc_camera *camera, const char *feature, int &value)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		auto c = reinterpret_cast<Camera *>(camera);

		if (c->features.count(feature) == 0) return PSINC_UNKNOWN_FEATURE;

		value = c->features[feature].Get();

		return PSINC_OK;
	}


	int psinc_camera_set_flash(psinc_camera *camera, unsigned char power)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		reinterpret_cast<Camera *>(camera)->SetFlash(power);

		return PSINC_OK;
	}


	int psinc_camera_set_context(psinc_camera *camera, unsigned char context)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		reinterpret_cast<Camera *>(camera)->SetContext(context);

		return PSINC_OK;
	}


	bool psinc_camera_connected(psinc_camera *camera)
	{
		if (!camera) return PSINC_INVALID_CAMERA;

		return reinterpret_cast<Camera *>(camera)->Connected();
	}
}
