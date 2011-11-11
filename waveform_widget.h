#ifndef JASS_WAVEFORM_WIDGET_HH
#define JASS_WAVEFORM_WIDGET_HH

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QVector>
#include <QPointF>
#include <QPalette>
#include <QPainter>
#include <QMouseEvent>

#include <cmath>
#include <algorithm>

#include "generator.h"

//! snap all sample/loop start/end points to the closest following zero crossing
inline void snap_to_zero(disposable_generator_ptr g) {
	double thresh = 0.001;
	unsigned int sample_length = g->t.sample_->t.data_0.size();
	unsigned int i;

	//! Adjust sample/loop start by finding zero crossing left of point
	unsigned int sstart = sample_length * g->t.sample_start;
	for (i = sstart; i >= 0; --i) {
		if (fabs(g->t.sample_->t.data_0[i]) < thresh) {
			break;
		}
	}
	g->t.sample_start = (double)i/sample_length;

	unsigned int lstart = sample_length * g->t.loop_start;
	for (i = lstart; i >= 0; --i) {
		if (fabs(g->t.sample_->t.data_0[i]) < thresh) {
			break;
		}
	}
	g->t.loop_start = (double)i/sample_length;

	//! Adjust sample/loop end by finding zero crossing right of point	
	unsigned int send = sample_length * g->t.sample_end;
	for (i = send; i < sample_length; ++i) {
		if (fabs(g->t.sample_->t.data_0[i]) < thresh) {
			break;
		}
	}
	g->t.sample_end = (double)i/sample_length;

	unsigned int lend = sample_length * g->t.loop_end;
	for (i = lend; i < sample_length; ++i) {
		if (fabs(g->t.sample_->t.data_0[i]) < thresh) {
			break;
		}
	}
	g->t.loop_end = (double)i/sample_length;

}

struct waveform_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	public:
		waveform_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			QWidget(parent),
			gen(gen)
		{
			QPalette palette = QWidget::palette();
			palette.setColor(backgroundRole(), Qt::white);
			setPalette(palette);
			setToolTip("Left-Click and drag to the right to sweep sample-start to sample-end. Shift-left-click and drag to the right to set loop-min to loop-max");
		}

		void paintEvent(QPaintEvent *)
		{
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.setPen(Qt::darkBlue);
			painter.setBrush(Qt::darkBlue);

			//! Sample waveform in n steps
			unsigned int n = 500;
			QVector<QPointF> points;
			points.push_back(QPointF(0.0, height()-1));
			for (unsigned int i = 0; i < n; ++i) {
				unsigned int sample_index = (gen->t.sample_->t.data_0.size() - 1)*(double(i)/(double)n);
				points.push_back(QPointF(width()*(double(i)/double(n)), height() * (1.0 - fabs(gen->t.sample_->t.data_0[sample_index]))));
			}
			painter.drawPolygon(&points[0], points.size(), Qt::OddEvenFill);

			//! Draw start/end range
			painter.setPen(QColor(0, 0, 128, 64));
			painter.setBrush(QColor(0, 0, 128, 64));
			painter.drawRect(width() * gen->t.sample_start, 0, width() * (gen->t.sample_end - gen->t.sample_start), height());
	
			if (gen->t.looping) {
				//! Draw loop range
				painter.setPen(QColor(128, 0, 0, 64));
				painter.setBrush(QColor(128, 0, 0, 64));
				painter.drawRect(width() * gen->t.loop_start, 0, width() * (gen->t.loop_end - gen->t.loop_start), height());
			}
		}


		void mouseMoveEvent(QMouseEvent *e) {
			if ((e->buttons() & Qt::LeftButton)) {
				if (e->modifiers() & Qt::ShiftModifier) {
					gen->t.loop_end = std::max(
								std::min((double)(e->x())/width(), gen->t.sample_end), 
								gen->t.loop_start
					);
					snap_to_zero(gen);
					e->accept();
					update();
				} else {
					gen->t.sample_end = std::max((double)(e->x())/width(), gen->t.sample_start);
					gen->t.loop_end = std::min(gen->t.sample_end, gen->t.loop_end);
					snap_to_zero(gen);
					e->accept();
					update();
				}
			}
		}

		void mousePressEvent(QMouseEvent *e) {
			if (e->button() == Qt::LeftButton) {
				if (e->modifiers() & Qt::ShiftModifier) {
					gen->t.loop_start = std::min(std::max((double)(e->x())/width(), gen->t.sample_start), gen->t.sample_end);
					snap_to_zero(gen);
					e->accept();
					update();
				} else {
					gen->t.sample_start = std::min((double)(e->x())/width(), gen->t.sample_end);
					gen->t.loop_start = std::max(gen->t.sample_start, gen->t.loop_start);
					snap_to_zero(gen);
					e->accept();
					update();
				}
			}
			if (e->button() == Qt::RightButton) {
				if (e->modifiers() & Qt::ShiftModifier) {
					gen->t.loop_end = std::max(std::min((double)(e->x())/width(), gen->t.sample_end), gen->t.sample_start);
					snap_to_zero(gen);
					e->accept();
					update();
				} else {
					gen->t.sample_end = std::max((double)(e->x())/width(), gen->t.sample_start);
					gen->t.loop_end = std::min(gen->t.sample_end, gen->t.loop_end);
					snap_to_zero(gen);
					e->accept();
					update();
				}
			}
		}



		QSize minimumSizeHint() const
		{
			return QSize(200, 10);
		}

		QSize sizeHint() const
		{
			return QSize(128 * 4, 10);
		}
};

#endif
