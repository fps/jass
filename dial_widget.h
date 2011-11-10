#ifndef JAZZ_DIAL_WIDGET_HH
#define JAZZ_DIAL_WIDGET_HH

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QSize>

#include <cmath>
#include <algorithm>

#include "generator.h"


struct dial_widget : public QWidget {
	Q_OBJECT

	double min_val, max_val, val;

	public:
		dial_widget(QWidget *parent = 0) :
			QWidget(parent),
			min_val(0),
			max_val(1),
			val(0)
		{

		}

		void paintEvent(QPaintEvent *)
		{
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);

			painter.setPen(QColor(0,0,0,255));
			painter.setBrush(QColor(0,0,0,255));
	
			QPen p = painter.pen();
			p.setWidth(2);
			painter.setPen(p);

			QRect r(0,0, 0, 0);
			r.setWidth(std::min(width(), height()) - 6);
			r.setHeight(std::min(width(), height()) - 6);	
			r.setX(3);
			r.setY(3);

			painter.drawArc(r, -16 * 60, 16 * 300);

			p.setWidth(5);
			painter.setPen(p);
			int angle = 300 * 16 - ((val-min_val)/(max_val-min_val) * 300 * 16) - 16 * 60;
			painter.drawArc(r, angle, 1);
		}

		void set_min_value(double v) {
			min_val = v;
		}

		void set_max_value(double v) {
			max_val = v;
		}

		double value() {
			return val;
		}

		void set_value(double v) {
			val = v;
			update();
		}

		void mouseMoveEvent(QMouseEvent *e) {
			if (e->buttons() & Qt::LeftButton) {
				QRect r(0,0, 0, 0);
				r.setWidth(std::min(width(), height()) - 6);
				r.setHeight(std::min(width(), height()) - 6);	
				r.setX(3);
				r.setY(3);

				double x,y;
				x = e->x() - r.width()/2 + 3;
				y = e->y() - r.height()/2 + 3;
				double angle =  fmod((270 + 180.0 * atan2(y,x)/M_PI), 360.0);
				//std::cout << angle << std::endl;
				val = min_val + (max_val - min_val) * (1.0/300.0) * (std::max(std::min(angle, 330.0), 30.0) - 30.0);
				std::cout << val << std::endl;
				emit valueChanged(val);
				e->accept();
				update();
			}
		}

		QSize minimumSizeHint() const
		{
			return QSize(10, 10);
		}

		QSize sizeHint() const
		{
			return QSize(30, 30);
		}
	signals:
		void valueChanged(double);

};

#endif

