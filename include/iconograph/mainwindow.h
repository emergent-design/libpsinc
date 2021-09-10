#pragma once

#include <QMainWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>

#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>
#include <chrono>
#include "settingsmodel.h"

//#ifndef __USE_GNU
//#define exp10(v) (std::pow(10, v))
//#endif

using emg::byte;
namespace Ui { class MainWindow; }


class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:

		void onConnection(bool connected);
		void onGrab(QImage *image);
		void onForceRestart();

		void on_canvas_updateInfo();

		void on_flashSlider_valueChanged(int value);
		void on_flashCheck_toggled(bool checked);
		void on_exposureSlider_valueChanged(int value);
		void on_exposureCheck_toggled(bool checked);
		void on_gainSlider_valueChanged(int value);
		void on_gainCheck_toggled(bool checked);
		void on_adcSlider_valueChanged(int value);
		void on_adcReset_clicked();
		void on_invertCheck_toggled(bool checked);
		void on_portraitCheck_toggled(bool checked);
		void on_compandingCheck_toggled(bool checked);
		void on_streamCheck_toggled(bool checked);
		void on_grabFrame_clicked();
		void on_modeBox_currentIndexChanged(int index);
		void on_contextSpin_valueChanged(int value);

		void on_setButton_clicked();
		void on_bitShiftBox_valueChanged(int value);

		void on_getButton_clicked();
		void on_hdrModeBox_currentIndexChanged(int index);

		void on_rangeWindowBox_valueChanged(int value);
		void on_rangeStartBox_valueChanged(int value);

		void on_saveHdrButton_clicked();
		void on_regionButton_clicked();

//		void on_lensCheck_toggled(bool checked);
//		void on_lensBox_valueChanged(double);

		void on_framerateSlider_valueChanged(int value);
		void on_framerateCheck_toggled(bool checked);
		void on_resetButton_clicked();

		void on_zoomButton_toggled(bool checked);
		void on_selectButton_toggled(bool checked);

		void on_spinBox_valueChanged(int value);

		void on_saveFrame_clicked();


		void on_faultResetButton_clicked();

signals:

		void connectionChanged(bool connected);
		void imageGrabbed(QImage *image);
		void forceRestart();

	private:

		enum class Hdr
		{
			Simple,
			Range,
			Window
		};

		struct FlashFault
		{
			int frames	= 0;
			int faults	= 0;
		};


		void Grab();
		void UpdateRegionInfo();
		void UpdateSlider(QSlider *slider, QSpinBox *spin, std::string alias);
		void UpdateCheck(QCheckBox *check, std::string alias);

		void UpdateUi();
		QImage *Convert();

		QImage *ConvertSimple();
		QImage *ConvertRange(int start, int window);
		QImage *ConvertWindow();

		Ui::MainWindow *ui;
		SettingsModel *settings;
		FeatureDelegate *delegate;

		psinc::Camera camera;
		psinc::ImageHandler<byte> handler;
		emg::Image<byte, emg::rgb> image;
		psinc::Camera::Mode mode = psinc::Camera::Mode::Normal;

		psinc::ImageHandler<uint16_t> hdrHandler;
		emg::Image<uint16_t, emg::rgb> hdrImage;
		emg::Buffer<int> histogram;


//		bool lensCorrect	= false;
//		emg::Image<byte, emg::rgb> corrected;
//		BrownsDistortion lens;
//		Map<byte> mapper;

		int frameCount		= 0;
		int droppedCount	= 0;
		byte context		= 0;
		bool stream			= true;
		bool connected		= false;
		bool portrait		= false;
		bool switchMode		= false;
		bool restartGrab	= false;
		bool save			= false;
        uint8_t usbVersion  = 0;

		Hdr hdrMode			= Hdr::Simple;
		int rangeStart		= 0;
		int rangeWindow		= 4096;

		std::chrono::time_point<std::chrono::steady_clock> last			= std::chrono::steady_clock::now();
		std::chrono::time_point<std::chrono::steady_clock> rateLast		= std::chrono::steady_clock::now();

		long rateLimit		= 40000;
		bool rateEnabled	= true;

		FlashFault flashFault;
};
