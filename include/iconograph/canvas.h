#pragma once

#include <QtWidgets/QLabel>
#include <QImage>
#include <QRect>


class Canvas : public QLabel
{
	Q_OBJECT

	public:

		Canvas(QWidget *parent = 0, Qt::WindowFlags f = 0);
//		~Canvas() {}


		void Update(QImage *image, bool portrait);
		QRect Roi();


	protected slots:

		virtual void paintEvent(QPaintEvent *);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *);

	private:

		QImage buffer;
		QRect roi		= { 0, 0, 0, 0 };
		QRect transient	= { 0, 0, 0, 0 };
		QRect rect		= { 0, 0, 0, 0 };
		double scale	= 1.0;
};

