#include "canvas.h"

#include <QPainter>
#include <QMouseEvent>
#include <iostream>
#include <cmath>

using namespace std;


Canvas::Canvas(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
}


void Canvas::Update(QImage *image, bool portrait)
{
	if (image)
	{
		this->buffer = portrait ? image->transformed(QTransform().rotate(-90)) : *image;
		this->setMaximumHeight(this->buffer.height());
	}

	this->repaint();
}


QRect Canvas::Roi()
{
	return this->roi.isEmpty() ? this->buffer.rect() : this->roi;
}


void Canvas::SetMode(Mode mode)
{
	this->mode = mode;

	switch (mode)
	{
		case Mode::Select:	this->zoom		= -1;
							this->offset	= { 0, 0 };
							break;

		case Mode::Zoom:	this->roi.setRect(0, 0, 0, 0);
							break;
	}
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
		this->minScale	= min(1.0, min((double)cw / (double)iw, (double)ch / (double)ih));

		if (this->zoom < 0 || this->mode != Mode::Zoom)
		{
			QImage image	= this->minScale < 1.0 ? this->buffer.scaledToWidth((int)(this->minScale * iw), Qt::SmoothTransformation) : this->buffer;
			int ox			= (cw - image.width()) / 2;
			int oy			= (ch - image.height()) / 2;

			painter.drawImage(ox, oy, image);

			this->rect.setRect(ox, oy, image.width(), image.height());
			this->scale = 1.0 / this->minScale;

			if (!this->roi.isEmpty())
			{
				painter.setPen(Qt::red);
				painter.drawRect(
					lrint(this->minScale * roi.left()) + ox, lrint(this->minScale * roi.top()) + oy,
					lrint(this->minScale * roi.width()), lrint(this->minScale * roi.height())
				);
			}
		}
		else
		{
			QImage image = this->zoom < 1.0 ? this->buffer.scaledToWidth((int)(this->zoom * iw), Qt::SmoothTransformation) : this->buffer;

			painter.drawImage(this->offset + this->transient.bottomRight() - this->transient.topLeft(), image);
		}
	}

	if (this->mode == Mode::Select && !this->transient.isEmpty())
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
	if (this->mode == Mode::Select)
	{
		if (this->transient.width() > 16 && this->transient.height() > 16)
		{
			auto r = this->rect.intersected(this->transient);
			r.translate(-this->rect.topLeft());

			this->roi.setRect(lrint(scale * r.left()), lrint(scale * r.top()), lrint(scale * r.width()), lrint(scale * r.height()));
		}
		else this->roi.setRect(0, 0, 0, 0);
	}
	else if (this->mode == Mode::Zoom)
	{
		this->offset = this->offset + this->transient.bottomRight() - this->transient.topLeft();
	}

	this->transient.setRect(0, 0, 0, 0);
	this->repaint();
}


void Canvas::wheelEvent(QWheelEvent *event)
{
	if (this->mode == Mode::Zoom)
	{
		if (this->zoom < 0)
		{
			this->zoom		= this->minScale;
			this->offset	= this->rect.topLeft();
		}

		// Origin of zoom in image space
		double x		= (event->x() - this->offset.x()) / this->zoom;
		double y		= (event->y() - this->offset.y()) / this->zoom;

		double step		= event->delta() < 0 ? -0.05 : 0.05;
		this->zoom		= std::max(this->minScale, std::min(1.0, this->zoom + step));
		this->offset	= {
			(int)lrint(event->x() - this->zoom * x),
			(int)lrint(event->y() - this->zoom * y)
		};
	}
}
