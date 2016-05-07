#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <emergent/logger/Logger.hpp>
//#include <entity/json.hpp>
#include <iostream>


using namespace std;
using namespace std::chrono;
using namespace psinc;
using namespace emergent;
//using namespace ent;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	Log::Initialise({ unique_ptr<logger::Sink>(new logger::Console()) });
	Log::Verbosity(Severity::Info);

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

	this->handler.Initialise(this->image);
	this->hdrHandler.Initialise(this->hdrImage);
	this->camera.Initialise("", [&](bool c) { emit connectionChanged(c); });

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
			*dst++ = Maths::clamp<byte>((256 * (*src++ - start)) / window);
			*dst++ = Maths::clamp<byte>((256 * (*src++ - start)) / window);
			*dst++ = Maths::clamp<byte>((256 * (*src++ - start)) / window);
		}
	}

	return result;
}


QImage *MainWindow::ConvertWindow()
{
	auto roi = this->ui->canvas->Roi();

	if (this->hdrImage.Size() < roi.width() * roi.height()) return nullptr;

	int x, y;
	int rw			= roi.width();
	int rh			= roi.height();
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
			this->histogram[*src++]++;
			this->histogram[*src++]++;
			this->histogram[*src++]++;
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

	return this->ConvertRange(low, std::max(256, high - low));
}



void MainWindow::Grab()
{
	auto *handler = this->hdrMode == Hdr::Simple ? (DataHandler *)&this->handler : (DataHandler *)&this->hdrHandler;

	this->camera.GrabImage(this->mode, *handler, [&](bool status) {

		if (status)
		{
			emit imageGrabbed(this->Convert());
		}

		if (this->restartGrab)
		{
			emit forceRestart();
			return false;
		}

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

		if (duration_cast<milliseconds>(steady_clock::now() - this->last).count() >= 5000)
		{
			this->ui->status->showMessage(QString::fromStdString(String::format("Camera connected (%.1f fps)", 0.2 * this->frameCount)));
			this->frameCount	= 0;
			this->last			= steady_clock::now();
		}
		else this->frameCount++;

		delete image;
	}
}


void MainWindow::onConnection(bool connected)
{
	if (connected != this->connected)
	{
		this->connected = connected;

		if (connected)
		{
			this->UpdateUi();

			this->ui->status->showMessage("Camera connected");
		}
		else this->ui->status->showMessage("Camera disconnected");

		this->settings->updateIndex();
	}
}


void MainWindow::UpdateSlider(QSlider *slider, QSpinBox *spin, string alias)
{
	auto &aliases = this->camera.aliases[this->context];

	if (aliases.count(alias))
	{
		slider->setEnabled(true);
		spin->setEnabled(true);
		slider->setRange(aliases[alias]->Minimum(), aliases[alias]->Maximum());
		spin->setRange(slider->minimum(), slider->maximum());
		slider->setValue(aliases[alias]->Get());
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

	if (aliases.count(alias))
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
		this->context = this->camera.aliases[0]["Context"]->Get();

		this->ui->contextSpin->setValue(this->context);

		this->UpdateSlider(this->ui->exposureSlider, this->ui->exposureSpin, "Exposure");
		this->UpdateCheck(this->ui->exposureCheck, "AutoExposure");
		this->UpdateSlider(this->ui->gainSlider, this->ui->gainSpin, "Gain");
		this->UpdateCheck(this->ui->gainCheck, "AutoGain");
		this->UpdateSlider(this->ui->adcSlider, this->ui->adcSpin, "ADCReference");
		this->UpdateCheck(this->ui->compandingCheck, "Companding");
	}
}


void MainWindow::Set(string alias, int value)
{
	if (this->camera.aliases[this->context].count(alias))
	{
		this->camera.aliases[this->context][alias]->Set(value);
	}
}

void MainWindow::Set(string alias, bool value)
{
	if (this->camera.aliases[this->context].count(alias))
	{
		this->camera.aliases[this->context][alias]->Set(value);
	}
}

int MainWindow::Reset(string alias)
{
	return this->camera.aliases[this->context].count(alias) ? this->camera.aliases[this->context][alias]->Reset() : 0;
}


void MainWindow::on_flashSlider_valueChanged(int value)		{ this->camera.SetFlash(value);}
void MainWindow::on_flashCheck_toggled(bool checked)		{ this->camera.SetFlash(checked ? this->ui->flashSlider->value() : 0); }
void MainWindow::on_exposureSlider_valueChanged(int value)	{ this->Set("Exposure", value); }
void MainWindow::on_exposureCheck_toggled(bool checked)		{ this->Set("AutoExposure", checked); }
void MainWindow::on_gainSlider_valueChanged(int value)		{ this->Set("Gain", value); }
void MainWindow::on_gainCheck_toggled(bool checked)			{ this->Set("AutoGain", checked); }
void MainWindow::on_adcSlider_valueChanged(int value)		{ this->Set("ADCReference", value); }
void MainWindow::on_compandingCheck_toggled(bool checked)	{ this->Set("Companding", checked); }
void MainWindow::on_adcReset_clicked()						{ this->ui->adcSlider->setValue(this->Reset("ADCReference")); }
void MainWindow::on_colourCheck_toggled(bool checked)		{ this->handler.Initialise(this->image, checked); }
void MainWindow::on_portraitCheck_toggled(bool checked)		{ this->portrait = checked; }
void MainWindow::on_grabFrame_clicked()						{ if (!this->stream) this->Grab(); }

void MainWindow::on_streamCheck_toggled(bool checked)
{
	this->stream = checked;

	if (checked) this->Grab();
}


void MainWindow::on_modeBox_currentIndexChanged(int index)
{
	this->mode = (Camera::Mode)index;

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
		this->hdrImage.SaveRaw("capture.eri");
	}
}


void MainWindow::on_regionButton_clicked()
{
	this->camera.SetWindow(0,
		this->ui->xRegionBox->value(), this->ui->yRegionBox->value(),
		this->ui->widthRegionBox->value(), this->ui->heightRegionBox->value()
	);
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
