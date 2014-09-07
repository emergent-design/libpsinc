#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <emergent/Logger.h>
#include <iostream>


using namespace std;
using namespace std::chrono;
using namespace psinc;
using namespace emergent;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	logger::instance().add(new sink::console());
	logger::instance().set_verbosity("info");

	connect(this, SIGNAL(connectionChanged(bool)), this, SLOT(onConnection(bool)));
	connect(this, SIGNAL(imageGrabbed(QImage*)), this, SLOT(onGrab(QImage*)));

	this->settings	= new SettingsModel(this, &this->camera);
	this->delegate	= new FeatureDelegate(this, this->settings);

	this->ui->setupUi(this);
	this->ui->advancedTable->setModel(this->settings);
	this->ui->advancedTable->setItemDelegate(this->delegate);
	this->ui->advancedTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	this->ui->advancedTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	this->handler.Initialise(this->image);
	this->camera.Initialise();

	this->camera.GrabImage(Camera::Mode::Master, this->handler, [&](int status) {
		switch (status)
		{
			case ACQUISITION_SUCCESSFUL: 			emit imageGrabbed(this->Convert());	break;
			case ACQUISITION_CONNECTED:				emit connectionChanged(true);		break;
			case ACQUISITION_DISCONNECTED:			emit connectionChanged(false);		break;
		}

		return this->stream;
	});
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
	int width		= this->image.Width();
	int height		= this->image.Height();
	int line		= width * this->image.Depth();
	byte *src		= this->image;
	auto *result	= new QImage(width, height, QImage::Format_RGB888);

	for (int y=0; y<height; y++, src+=line)
	{
		memcpy(result->scanLine(y), src, line);
	}

	return result;
}


void MainWindow::onGrab(QImage *image)
{
	if (this->portrait)
	{
		*image = image->transformed(QTransform().rotate(-90));
	}

	int cw			= this->ui->canvas->width();
	int ch			= this->ui->canvas->height();
	int iw			= image->width();
	int ih			= image->height();
	double scale	= min(1.0, min((double)cw / (double)iw, (double)ch / (double)ih));

	this->ui->canvas->setPixmap(QPixmap::fromImage(
		scale < 1.0	? image->scaledToWidth((int)(scale * iw), Qt::SmoothTransformation)	: *image
	));

	this->ui->canvas->setMaximumHeight(ih);

	if (duration_cast<milliseconds>(steady_clock::now() - this->last).count() >= 5000)
	{
		this->ui->status->showMessage(QString::fromStdString(tfm::format("Camera connected (%.1f fps)", 0.2 * this->frameCount)));
		this->frameCount	= 0;
		this->last			= steady_clock::now();
	}
	else this->frameCount++;

	delete image;
}


void MainWindow::onConnection(bool connected)
{
	if (connected != this->connected)
	{
		this->connected = connected;

		if (connected)
		{
			this->ui->exposureSlider->setValue(this->camera.aliases[0]["Exposure"]->Get());
			this->ui->exposureCheck->setChecked(this->camera.aliases[0]["AutoExposure"]->Get());
			this->ui->gainSlider->setValue(this->camera.aliases[0]["Gain"]->Get());
			this->ui->gainCheck->setChecked(this->camera.aliases[0]["AutoGain"]->Get());
			this->ui->adcSlider->setValue(this->camera.aliases[0]["ADCReference"]->Get());
			this->ui->compandingCheck->setChecked(this->camera.aliases[0]["Companding"]->Get() == 3);

			this->ui->status->showMessage("Camera connected");
		}
		else this->ui->status->showMessage("Camera disconnected");

		this->settings->updateIndex();
	}
}


void MainWindow::on_flashSlider_valueChanged(int value)		{ this->camera.SetFlash(value);}
void MainWindow::on_flashCheck_toggled(bool checked)		{ this->camera.SetFlash(checked ? this->ui->flashSlider->value() : 0); }
void MainWindow::on_exposureSlider_valueChanged(int value)	{ this->camera.aliases[0]["Exposure"]->Set(value); }
void MainWindow::on_exposureCheck_toggled(bool checked)		{ this->camera.aliases[0]["AutoExposure"]->Set(checked ? 1 : 0); }
void MainWindow::on_gainSlider_valueChanged(int value)		{ this->camera.aliases[0]["Gain"]->Set(value); }
void MainWindow::on_gainCheck_toggled(bool checked)			{ this->camera.aliases[0]["AutoGain"]->Set(checked ? 1 : 0); }
void MainWindow::on_adcSlider_valueChanged(int value)		{ this->camera.aliases[0]["ADCReference"]->Set(value); }
void MainWindow::on_compandingCheck_toggled(bool checked)	{ this->camera.aliases[0]["Companding"]->Set(checked ? 3 : 2); }
void MainWindow::on_adcReset_clicked()						{ this->ui->adcSlider->setValue(this->camera.aliases[0]["ADCReference"]->Reset()); }
void MainWindow::on_colourCheck_toggled(bool checked)		{ this->handler.Initialise(this->image, checked); }
void MainWindow::on_portraitCheck_toggled(bool checked)		{ this->portrait = checked; }
