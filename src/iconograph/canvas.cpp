#include "canvas.h"

#include <QPainter>
#include <QMouseEvent>
#include <iostream>

using namespace std;


Canvas::Canvas(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
}


void Canvas::Update(QImage *image, bool portrait)
{
	if (image)
	{
		this->buffer = portrait ? image->transformed(QTransform().rotate(-90)) : *image;

		//	int cw			= this->width();
		//	int ch			= this->height();
		//	int iw			= image->width();
		//	int ih			= image->height();
		//	double scale	= min(1.0, min((double)cw / (double)iw, (double)ch / (double)ih));

		//	this->setPixmap(QPixmap::fromImage(
		//		scale < 1.0	? image->scaledToWidth((int)(scale * iw), Qt::SmoothTransformation)	: *image
		//	));

		this->setMaximumHeight(this->buffer.height());
	}

	this->repaint();
}


QRect Canvas::Roi()
{
	return this->roi.isEmpty() ? this->buffer.rect() : this->roi;
}


void Canvas::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	if (this->buffer.width())
	{
		int cw			= this->width();
		int ch			= this->height();
		int iw			= this->buffer.width();
		int ih			= this->buffer.height();
		double scale	= min(1.0, min((double)cw / (double)iw, (double)ch / (double)ih));
		QImage image	= scale < 1.0 ? this->buffer.scaledToWidth((int)(scale * iw), Qt::SmoothTransformation) : this->buffer;
		int ox			= (cw - image.width()) / 2;
		int oy			= (ch - image.height()) / 2;

		painter.drawImage(ox, oy, image);

		this->rect.setRect(ox, oy, image.width(), image.height());
		this->scale = 1.0 / scale;

		if (!this->roi.isEmpty())
		{
			painter.setPen(Qt::red);
			painter.drawRect(lrint(scale * roi.left()) + ox, lrint(scale * roi.top()) + oy, lrint(scale * roi.width()), lrint(scale * roi.height()));
		}
	}

	if (!this->transient.isEmpty())
	{
		painter.setPen(Qt::blue);
		painter.drawRect(this->transient);
	}
}


void Canvas::mouseMoveEvent(QMouseEvent *event)
{
	this->transient.setBottomRight(event->pos());
}


void Canvas::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->transient.setTopLeft(event->pos());
		this->transient.setSize({ 1, 1 });
	}
}


void Canvas::mouseReleaseEvent(QMouseEvent *)
{
	if (this->transient.width() > 16 && this->transient.height() > 16)
	{
		auto r = this->rect.intersected(this->transient);
		r.translate(-this->rect.topLeft());

		this->roi.setRect(lrint(scale * r.left()), lrint(scale * r.top()), lrint(scale * r.width()), lrint(scale * r.height()));
	}
	else this->roi.setRect(0, 0, 0, 0);

	this->transient.setRect(0, 0, 0, 0);
	this->repaint();
}
