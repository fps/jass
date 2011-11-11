#ifndef JASS_VELOCITY_RANGE_WIDGET_HH
#define JASS_VELOCITY_RANGE_WIDGET_HH

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

#include "generator.h"

struct velocity_range_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	public:
		velocity_range_widget(disposable_generator_ptr g, QWidget *parent = 0) :
			QWidget(parent),
			gen(g) 
		{
			setToolTip("Left click to set minimum velocity. Right click to set maximum. Left Click and Drag to the right to sweep a velocity range (0..127) which this generator responds to");
		}

		void paintEvent(QPaintEvent *)
		{
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);

			painter.setPen(QColor(0, 0, 128, 64));
			painter.setBrush(QColor(0,0, 128, 64));

			painter.drawRect(width() * gen->t.min_velocity/128.0, 0, width() * (gen->t.max_velocity - gen->t.min_velocity + 1)/128.0, height());
		}

		void mouseMoveEvent(QMouseEvent *e) {
			if ((e->buttons() & Qt::LeftButton)) {
				gen->t.max_velocity = std::max((unsigned int)((double)(e->x())/width() * 128), gen->t.min_velocity);
				e->accept();
				update();
			}
		}

		void mousePressEvent(QMouseEvent *e) {
			if (e->button() == Qt::LeftButton) {
				gen->t.min_velocity = std::min((double)(e->x())/width() * 128, (double)(gen->t.max_velocity));
				e->accept();
				update();
			}
			if (e->button() == Qt::RightButton) {
				gen->t.max_velocity = std::max((double)(e->x())/width() * 128, (double)(gen->t.min_velocity));
				e->accept();
				update();
			}
		}


};

#endif

