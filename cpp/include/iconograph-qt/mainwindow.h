#pragma once

#include <QMainWindow>

#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.h>
#include <chrono>
#include "settingsmodel.h"


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

		void on_flashSlider_valueChanged(int value);
		void on_flashCheck_toggled(bool checked);
		void on_exposureSlider_valueChanged(int value);
		void on_exposureCheck_toggled(bool checked);
		void on_gainSlider_valueChanged(int value);
		void on_gainCheck_toggled(bool checked);
		void on_adcSlider_valueChanged(int value);
		void on_adcReset_clicked();
		void on_colourCheck_toggled(bool checked);
		void on_portraitCheck_toggled(bool checked);
		void on_compandingCheck_toggled(bool checked);
		void on_streamCheck_toggled(bool checked);
		void on_grabFrame_clicked();

	signals:

		void connectionChanged(bool connected);
		void imageGrabbed(QImage *image);

	private:


		void Grab();
		QImage *Convert();

		Ui::MainWindow *ui;
		SettingsModel *settings;
		FeatureDelegate *delegate;

		psinc::Camera camera;
		psinc::ImageHandler handler;
		emg::Image<byte, emg::rgb> image;

		int frameCount	= 0;
		bool stream		= true;
		bool connected	= false;
		bool portrait	= false;

		std::chrono::time_point<std::chrono::steady_clock> last = std::chrono::steady_clock::now();
};
