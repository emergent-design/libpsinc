#include "psinc/handlers/ImageHandler.h"

using namespace std;
using namespace emergent;


namespace psinc
{
	ImageHandler::ImageHandler(ImageBase<byte> &image)
	{
		this->image = &image;
	}


	void ImageHandler::Initialise(ImageBase<byte> &image)
	{
		this->image = &image;
	}


	bool ImageHandler::Process(bool monochrome, emg::Buffer<byte> &data, int width, int height)
	{
		// Allow the monochrome flag to be overridden - could have unexpected effects.
		if (this->forceBayer) monochrome = false;

		int w = monochrome ? width : width - 4;
		int h = monochrome ? height : height - 4;

		if (w > 0 && h > 0 && data.Size() == width * height)
		{
			this->image->Resize(w, h);

			if (monochrome)						this->DecodeMono(data, this->image->Data(), width, height, this->image->Depth());
			else if (this->image->Depth() == 3)	this->DecodeColour(data, this->image->Data(), width, height);
			else								this->DecodeGrey(data, this->image->Data(), width, height);

			return true;
		}

		return false;
	}
}

