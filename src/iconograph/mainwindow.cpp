#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <emergent/logger/Logger.hpp>
#include <iostream>


//using namespace std;
//using namespace std::chrono;
//using namespace psinc;
//using namespace emergent;
//using namespace ent;
using std::string;
using emg::byte;
using emg::Log;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	#ifdef __linux__
		Log::Initialise({ std::unique_ptr<emg::logger::Sink>(new emg::logger::Console()) });
	#else
		Log::Initialise({ std::unique_ptr<emg::logger::Sink>(new emg::logger::LogFile("iconograph.log")) });
	#endif

	Log::Verbosity(emg::Severity::Info);

	connect(this, SIGNAL(connectionChanged(bool)), this, SLOT(onConnection(bool)));
	connect(this, SIGNAL(imageGrabbed(QImage*)), this, SLOT(onGrab(QImage*)));
	connect(this, SIGNAL(forceRestart()), this, SLOT(onForceRestart()));

	this->settings	= new SettingsModel(this, &this->camera);
	this->delegate	= new FeatureDelegate(this, this->settings);

	this->ui->setupUi(this);
	this->ui->advancedTable->setModel(this->settings);
	this->ui->advancedTable->setItemDelegate(this->delegate);
	this->ui->advancedTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	this->ui->advancedTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->ui->tabs->setCurrentIndex(0);

	this->handler.Initialise(this->image);
	this->hdrHandler.Initialise(this->hdrImage);
    this->camera.Initialise("", [&](bool c) { emit connectionChanged(c); }, 200);

	this->Grab();
}



MainWindow::~MainWindow()
{
	this->stream = false;
	while (this->camera.Grabbing());

	delete this->ui;
	delete this->delegate;
	delete this->settings;
}


QImage *MainWindow::Convert()
{
	switch (this->hdrMode)
	{
		case Hdr::Simple:		return this->ConvertSimple();
		case Hdr::Range:		return this->ConvertRange(this->rangeStart, this->rangeWindow);
		case Hdr::Window:		return this->ConvertWindow();
		default:				return nullptr;
	}
}


QImage *MainWindow::ConvertSimple()
{
	int width		= this->image.Width();
	int height		= this->image.Height();
	int line		= width * 3;
	byte *src		= this->image;
	auto *result	= new QImage(width, height, QImage::Format_RGB888);

//	if (this->lensCorrect)
//	{
//		this->mapper.Transform(this->corrected);
//		src = this->corrected;
//	}

	for (int y=0; y<height; y++, src+=line)
	{
		memcpy(result->scanLine(y), src, line);
	}

	return result;
}


QImage *MainWindow::ConvertRange(int start, int window)
{
	int x, y;
	int width		= this->hdrImage.Width();
	int height		= this->hdrImage.Height();
	uint16_t *src	= this->hdrImage;
	auto *result	= new QImage(width, height, QImage::Format_RGB888);
	byte *dst;

	for (y=0; y<height; y++)
	{
		dst = result->scanLine(y);

		for (x=0; x<width; x++)
		{
			// mt9 camera uses the upper 12-bits so bitshift the value first
			*dst++ = emg::Maths::clamp<byte>((256 * ((*src++ >> 4) - start)) / window);
			*dst++ = emg::Maths::clamp<byte>((256 * ((*src++ >> 4) - start)) / window);
			*dst++ = emg::Maths::clamp<byte>((256 * ((*src++ >> 4)- start)) / window);
		}
	}

	return result;
}


QImage *MainWindow::ConvertWindow()
{
	auto roi = this->ui->canvas->Roi();

	if (this->hdrImage.Size() < roi.width() * roi.height())
	{
		return nullptr;
	}

	int x, y;
	int rw			= roi.isEmpty() ? this->hdrImage.Width() : roi.width();
	int rh			= roi.isEmpty() ? this->hdrImage.Height() : roi.height();
	int width		= this->hdrImage.Width();
	int size		= rw * rh * 3;
	int jump		= (width - rw) * 3;
	uint16_t *src	= this->hdrImage + roi.top() * width * 3 + roi.left() * 3;

	this->histogram.Resize(4096);
	this->histogram.Clear();

	for (y=0; y<rh; y++, src+=jump)
	{
		for (x=0; x<rw; x++)
		{
			// mt9 camera uses the upper 12-bits so bitshift the value first
			this->histogram[*src++ >> 4]++;
			this->histogram[*src++ >> 4]++;
			this->histogram[*src++ >> 4]++;
		}
	}

	// Top and bottom 1%
	int sum;
	int threshold	= lrint(0.01 * size);
	int low			= 0;
	int high		= 4095;

	for (sum=0; sum < threshold && low < 4095; low++)
	{
		sum += this->histogram[low];
	}
	for (sum=0; sum < threshold && high > 0; high--)
	{
		sum += this->histogram[high];
	}

	return this->ConvertRange(low, high - low); //std::max(256, high - low));
}



void MainWindow::Grab()
{
	auto *handler = this->hdrMode == Hdr::Simple
			? (psinc::DataHandler *)&this->handler
			: (psinc::DataHandler *)&this->hdrHandler;

	this->camera.GrabImage(this->mode, *handler, [&](bool status) {

		if (status)
		{
			if (this->save)
			{
				if (this->hdrMode == Hdr::Simple)
				{
					this->image.Save(emg::String::format("grab_%s.png", emg::Timestamp::FNow()));
				}
				else
				{
					this->hdrImage.Save(emg::String::format("grab_hdr_%s.png", emg::Timestamp::FNow()));
				}
				this->save = false;
			}

			emit imageGrabbed(this->Convert());
		}
		else
		{
			this->droppedCount++;
		}

		if (this->restartGrab)
		{
			emit forceRestart();
			return false;
		}

		if (this->stream && this->rateEnabled)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(std::max(0l,
				this->rateLimit - (long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - this->rateLast).count()
			)));

			this->rateLast = std::chrono::steady_clock::now();
		}

		// Switch contexts each frame
//		this->context = this->context ? 0 : 1;
//		this->camera.SetContext(this->context);

		return this->stream;
	});
}


void MainWindow::onForceRestart()
{
	this->restartGrab = false;
	this->Grab();
}


void MainWindow::onGrab(QImage *image)
{
	if (image)
	{
		this->ui->canvas->Update(image, this->portrait);
		this->on_canvas_updateInfo();
		this->UpdateRegionInfo();

		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->last).count() >= 5000)
		{
			this->ui->status->showMessage(QString::fromStdString(emg::String::format(
                "Camera connected - USB%d  - %.1f fps - %d dropped",
                this->usbVersion, 0.2 * this->frameCount, this->droppedCount
			)));

			this->frameCount	= 0;
			this->last			= std::chrono::steady_clock::now();
		}
		else this->frameCount++;

		delete image;
	}
}


void MainWindow::onConnection(bool connected)
{
	if (connected != this->connected)
	{
		this->droppedCount	= 0;
		this->connected		= connected;

		if (connected)
		{
            this->usbVersion = this->camera.UsbVersion();
			this->UpdateUi();

			this->ui->status->showMessage("Camera connected");
		}
		else this->ui->status->showMessage("Camera disconnected");

		this->settings->updateIndex();
	}
}


void MainWindow::on_canvas_updateInfo()
{
	const auto pos = this->ui->canvas->Position();

	const bool hdr		= this->hdrMode == Hdr::Simple;
	const int width		= hdr ? this->image.Width() : this->hdrImage.Width();
	const int height	= hdr ? this->image.Height() : this->hdrImage.Height();
	const int x			= std::max(0, std::min(width-1, pos.x()));
	const int y			= std::max(0, std::min(height-1, pos.y()));
	const int offset	= y * width * 3 + x * 3;

	if (!width || !height)
	{
		return;
	}

	const int r	= hdr ? image.Data()[offset] : hdrImage.Data()[offset];
	const int g = hdr ? image.Data()[offset+1] : hdrImage.Data()[offset+1];
	const int b = hdr ? image.Data()[offset+2] : hdrImage.Data()[offset+2];

	this->ui->currentX->setValue(x);
	this->ui->currentY->setValue(y);
	this->ui->currentR->setValue(r);
	this->ui->currentG->setValue(g);
	this->ui->currentB->setValue(b);
	this->ui->currentV->setValue((r + g + b) / 3);
}

template <typename T> std::array<int64_t, 7> GetRegionMeans(const emg::Image<T, emg::rgb> &image, const QRect &roi)
{
	const int width		= image.Width();
	const int height	= image.Height();

	if (!width || !height)
	{
		return { 0 };
	}

	const int sx	= roi.x();
	const int sy	= roi.y();
	const int jump	= 3 * (width - roi.width());
	auto sum		= std::array<int64_t, 3>{ 0, 0, 0 };
	auto bayer		= std::array<int64_t, 4> { 0, 0, 0, 0 };
	auto tally		= std::array<int, 4> { 0, 0, 0, 0 };
	T *src			= image + sy * width * 3 + sx * 3;

	for (int y=0; y<roi.height(); y++, src+=jump)
	{
		// If the current row in the full image is even or odd then calculate
		// the appropriate offset in the bayer value array
		const int offset = 2 * ((sy + y) % 2);

		for (int x=0; x<roi.width(); x++, src+=3)
		{
			int value = 0;
			for (int j=0; j<3; j++)
			{
				sum[j] += src[j];
				value += src[j];
			}

			// 0 1
			// 2 3
			bayer[offset + ((sx + x) % 2)] += value / 3;
			tally[offset + ((sx + x) % 2)]++;
		}
	}

	const int size = roi.width() * roi.height();

	return {
		sum[0] / size,	// r
		sum[1] / size,	// g
		sum[2] / size,	// b
		tally[0] ? bayer[0] / tally[0] : 0,
		tally[1] ? bayer[1] / tally[1] : 0,
		tally[2] ? bayer[2] / tally[2] : 0,
		tally[3] ? bayer[3] / tally[3] : 0,
	};
}


void MainWindow::UpdateRegionInfo()
{
	const auto roi	= this->ui->canvas->Roi();
	auto values		= roi.isEmpty()
			? std::array<int64_t, 7> { 0 }
			: this->hdrMode == Hdr::Simple
				? GetRegionMeans(this->image, roi)
				: GetRegionMeans(this->hdrImage, roi);

	const int intensity = (values[0] + values[1] + values[2]) / 3;

	this->ui->regionR->setValue(values[0]);
	this->ui->regionG->setValue(values[1]);
	this->ui->regionB->setValue(values[2]);
	this->ui->regionV->setValue(intensity);

	this->ui->bayer0->setValue(values[3]);
	this->ui->bayer1->setValue(values[4]);
	this->ui->bayer2->setValue(values[5]);
	this->ui->bayer3->setValue(values[6]);

	// Update flash fault stats
	const int threshold = this->ui->faultThreshold->value();

	if (threshold)
	{
		this->flashFault.frames++;
		if (intensity < threshold)
		{
			this->flashFault.faults++;
		}
	}
	else
	{
		this->flashFault.frames = this->flashFault.faults = 0;
	}

	this->ui->frameCountTotal->display(this->flashFault.frames);
	this->ui->frameCountFault->display(this->flashFault.faults);
}


void MainWindow::UpdateSlider(QSlider *slider, QSpinBox *spin, string alias)
{
	auto &aliases = this->camera.aliases[this->context];

	if (aliases.Contains(alias))
	{
		int value = aliases[alias]->Get();

		slider->blockSignals(true);
		spin->blockSignals(true);
			slider->setEnabled(true);
			spin->setEnabled(true);
			slider->setRange(aliases[alias]->Minimum(), aliases[alias]->Maximum());
			spin->setRange(slider->minimum(), slider->maximum());
			slider->setValue(value);
			spin->setValue(value);
		slider->blockSignals(false);
		spin->blockSignals(false);
	}
	else
	{
		slider->setEnabled(false);
		spin->setEnabled(false);
	}
}


void MainWindow::UpdateCheck(QCheckBox *check, string alias)
{
	auto &aliases = this->camera.aliases[this->context];

	if (aliases.Contains(alias))
	{
		check->setEnabled(true);
		check->setChecked(aliases[alias]->Get() == aliases[alias]->Maximum());
	}
	else check->setEnabled(false);
}


void MainWindow::UpdateUi()
{
	if (this->connected)
	{
		this->context = this->camera.aliases[0].context->Get();

		this->ui->contextSpin->setValue(this->context);

		this->UpdateSlider(this->ui->exposureSlider, this->ui->exposureSpin, "Exposure");
		this->UpdateCheck(this->ui->exposureCheck, "AutoExposure");
		this->UpdateSlider(this->ui->gainSlider, this->ui->gainSpin, "Gain");
		this->UpdateCheck(this->ui->gainCheck, "AutoGain");
		this->UpdateSlider(this->ui->adcSlider, this->ui->adcSpin, "ADCReference");
		this->UpdateCheck(this->ui->compandingCheck, "Companding");
	}
}


void MainWindow::on_flashSlider_valueChanged(int value)		{ camera.SetFlash(value);}
void MainWindow::on_flashCheck_toggled(bool checked)		{ camera.SetFlash(checked ? ui->flashSlider->value() : 0); }
void MainWindow::on_exposureSlider_valueChanged(int value)	{ camera.aliases[context].exposure->Set(value); }
void MainWindow::on_exposureCheck_toggled(bool checked)		{ camera.aliases[context].autoExposure->Set(checked); }
void MainWindow::on_gainSlider_valueChanged(int value)		{ camera.aliases[context].gain->Set(value); }
void MainWindow::on_gainCheck_toggled(bool checked)			{ camera.aliases[context].autoGain->Set(checked); }
void MainWindow::on_adcSlider_valueChanged(int value)		{ camera.aliases[context].adcReference->Set(value); }
void MainWindow::on_compandingCheck_toggled(bool checked)	{ camera.aliases[context].companding->Set(checked); }
void MainWindow::on_adcReset_clicked()						{ ui->adcSlider->setValue(camera.aliases[context].adcReference->Reset()); }
void MainWindow::on_invertCheck_toggled(bool checked)		{ handler.Initialise(this->image, checked); }
void MainWindow::on_portraitCheck_toggled(bool checked)		{ portrait = checked; }
void MainWindow::on_grabFrame_clicked()						{ if (!this->stream) this->Grab(); }
void MainWindow::on_framerateSlider_valueChanged(int value)	{ rateLimit = lrint(1000000.0 / value); }
void MainWindow::on_framerateCheck_toggled(bool checked)	{ rateEnabled = checked; }
void MainWindow::on_zoomButton_toggled(bool checked)		{ if (checked) ui->canvas->SetMode(Canvas::Mode::Zoom); }
void MainWindow::on_selectButton_toggled(bool checked)		{ if (checked) ui->canvas->SetMode(Canvas::Mode::Select); }

void MainWindow::on_streamCheck_toggled(bool checked)
{
	this->stream = checked;

	if (checked) this->Grab();
}


void MainWindow::on_modeBox_currentIndexChanged(int index)
{
	this->mode = (psinc::Camera::Mode)index;

	if (this->stream) this->ui->streamCheck->toggle();
}


void MainWindow::on_contextSpin_valueChanged(int value)
{
	this->camera.SetContext(value);
	this->UpdateUi();
}


void MainWindow::on_getButton_clicked()
{
	this->ui->valueBox->setValue(
		this->camera.GetRegister(this->ui->addressBox->value())
	);
}


void MainWindow::on_setButton_clicked()
{
	this->camera.SetRegister(
		this->ui->addressBox->value(),
		this->ui->valueBox->value()
	);
}


void MainWindow::on_bitShiftBox_valueChanged(int value)
{
	this->handler.Shift(value);
}


void MainWindow::on_hdrModeBox_currentIndexChanged(int index)
{
	this->hdrMode = (Hdr)index;

	if (this->stream) this->restartGrab = true;

	this->ui->bitShiftBox->setEnabled(this->hdrMode == Hdr::Simple);
	this->ui->rangeStartSlider->setEnabled(this->hdrMode == Hdr::Range);
	this->ui->rangeStartBox->setEnabled(this->hdrMode == Hdr::Range);
	this->ui->rangeWindowSlider->setEnabled(this->hdrMode == Hdr::Range);
	this->ui->rangeWindowBox->setEnabled(this->hdrMode == Hdr::Range);
	this->ui->saveHdrButton->setEnabled(this->hdrMode != Hdr::Simple);
}


void MainWindow::on_rangeWindowBox_valueChanged(int value)
{
	this->rangeWindow	= value;
	int maximum			= this->ui->rangeWindowBox->maximum() - value;
	this->ui->rangeStartBox->setMaximum(maximum);
	this->ui->rangeStartSlider->setMaximum(maximum);
}


void MainWindow::on_rangeStartBox_valueChanged(int value)
{
	this->rangeStart = value;
}


void MainWindow::on_saveHdrButton_clicked()
{
	if (this->hdrImage.Size())
	{
		auto path = QFileDialog::getSaveFileName(this, "Save image", QDir::currentPath(), "PNG image (*.png);;Emergent raw image (*.eri)");

		if (path.endsWith("eri"))
		{
			this->hdrImage.SaveRaw(path.toStdString());
		}
		else
		{
			this->hdrImage.Save(path.toStdString());
		}
	}
}

void MainWindow::on_saveFrame_clicked()
{
	this->save = true;
}


void MainWindow::on_regionButton_clicked()
{
	if (!this->camera.SetWindow(0,
		this->ui->xRegionBox->value(), this->ui->yRegionBox->value(),
		this->ui->widthRegionBox->value(), this->ui->heightRegionBox->value()
	))
	{
		Log::Error("Failed to set window region parameters");
	}
}


void MainWindow::on_resetButton_clicked()
{
	switch (this->ui->resetBox->currentIndex())
	{
		case 0:	this->camera.Reset(psinc::ResetLevel::Connection);						break;
		case 1:	this->camera.Reset(psinc::ResetLevel::Control);							break;
		case 2:	this->camera.Reset(psinc::ResetLevel::Command);							break;
		case 3:	this->camera.Reset(psinc::ResetLevel::Communications);					break;
		case 4:	this->camera.Reset(psinc::ResetLevel::Imaging);		this->UpdateUi();	break;
		case 5:	this->camera.Reset(psinc::ResetLevel::ImagingSoft);	this->UpdateUi();	break;
		case 6:	this->camera.Reset(psinc::ResetLevel::Io);								break;
	}
}


void MainWindow::on_spinBox_valueChanged(int value)
{
	this->camera.SetTimeout(value);
}


void MainWindow::on_faultResetButton_clicked()
{
	this->flashFault.frames = this->flashFault.faults = 0;
}


//void MainWindow::on_lensCheck_toggled(bool checked)
//{
//	if (checked)
//	{
//		int width	= this->image.Width();
//		int height	= this->image.Height();

//		entity::decode<json>(String::format(u8R"json({
//			"forward": { "radial": [ 0.0 ], "centre": { "x" : 0, "y": 0 }},
//			"reverse": { "radial": [ %f ], "centre": { "x" : 0, "y": 0 }},
//			"scale": 1.0, "origin": { "x": %f, "y": %f }, "width": %d, "height": %d
//		})json", this->ui->lensBox->value(), 0.5 * width, 0.5 * height, width, height), this->lens);

//		this->mapper.Initialise(this->image, width, height, this->lens);
//	}

//	this->lensCorrect = checked;
//}


//void MainWindow::on_lensBox_valueChanged(double)
//{
//	this->ui->lensCheck->setChecked(false);
//}





