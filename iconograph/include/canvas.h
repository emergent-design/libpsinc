#pragma once

#include <QtWidgets/QLabel>
#include <QImage>
#include <QRect>


class Canvas : public QLabel
{
	Q_OBJECT

	public:

		enum class Mode
		{
			Zoom,
			Select
		};

		Canvas(QWidget *parent = 0, Qt::WindowFlags f = {});
//		~Canvas() {}


		void Update(QImage *image, bool portrait);
		QRect Roi();
		QPoint Position();


		void SetMode(Mode mode);

	signals:
		void updateInfo();

	protected slots:

		virtual void paintEvent(QPaintEvent *);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *);
		virtual void wheelEvent(QWheelEvent *event);

	private:

		QImage buffer;
		QRect roi		= { 0, 0, 0, 0 };
		QRect transient	= { 0, 0, 0, 0 };
		QRect rect		= { 0, 0, 0, 0 };
		double scale	= 1.0;

		// Corner offset when panning the image
		QPoint offset	= { 0, 0 };
		double minScale	= 0.0;
		double zoom		= -1;	// <0 = fill canvas

		QPoint position = { 0, 0 };

		Mode mode		= Mode::Zoom;
};

