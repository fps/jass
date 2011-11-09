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

#include <cmath>

#include "generator.h"

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
			painter.drawRect(width() * gen->t.sample_start/gen->t.sample_->t.data_0.size(), 0, width() * gen->t.sample_end/gen->t.sample_->t.data_0.size(), height());
	
			//! Draw loop range
			painter.setPen(QColor(128, 0, 0, 64));
			painter.setBrush(QColor(128, 0, 0, 64));
			painter.drawRect(width() * gen->t.loop_start/gen->t.sample_->t.data_0.size(), 0, width() * gen->t.loop_end/gen->t.sample_->t.data_0.size(), height());
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
