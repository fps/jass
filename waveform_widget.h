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
#include "engine.h"


struct waveform_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	public:
//! snap all sample/loop start/end points to the closest following zero crossing
		inline void snap_to_zero(double sample_start, double sample_end, double loop_start, double loop_end) {
			double thresh = 0.001;
			unsigned int sample_length = gen->t.sample_->t.data_0.size();
			unsigned int i;

	

			//! Adjust sample/loop start by finding zero crossing left of point
			unsigned int sstart = sample_length * sample_start;

			for (i = sstart; i >= 0; --i) {
				if (fabs(gen->t.sample_->t.data_0[i]) < thresh) {
					break;
				}
			}
			sample_start = (double)i/sample_length;

			unsigned int lstart = sample_length * loop_start;

			for (i = lstart; i >= 0; --i) {
				if (fabs(gen->t.sample_->t.data_0[i]) < thresh) {
					break;
				}
			}
			loop_start = (double)i/sample_length;
		
			//! Adjust sample/loop end by finding zero crossing right of point	
			unsigned int send = sample_length * sample_end;

			for (i = send; i < sample_length; ++i) {
				if (fabs(gen->t.sample_->t.data_0[i]) < thresh) {
					break;
				}
			}
			sample_end = (double)i/sample_length;
		
			unsigned int lend = sample_length * loop_end;

			for (i = lend; i < sample_length; ++i) {
				if (fabs(gen->t.sample_->t.data_0[i]) < thresh) {
					break;
				}
			}
			loop_end = (double)i/sample_length;
		
			engine::get()->write_command(assign(gen->t.sample_start, sample_start));
			engine::get()->write_command(assign(gen->t.sample_end, sample_end));
			engine::get()->write_command(assign(gen->t.loop_start, loop_start));
			engine::get()->write_command(assign(gen->t.loop_end, loop_end));
			engine::get()->deferred_commands.write(boost::bind(&waveform_widget::update, this));
		}
		
		waveform_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			QWidget(parent),
			gen(gen)
		{
			QPalette palette = QWidget::palette();
			palette.setColor(backgroundRole(), Qt::white);
			setPalette(palette);
			setToolTip("Left-click to set sample start. Right-click to set sample end. Left-click and drag to sweep sample-start to sample-end.\n Shift-left-click to set loop start. Shift-right-click to set loop end. Shift-left-click and drag to the right to set loop-min to loop-max");
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
					double loop_end = std::max(
								std::min((double)(e->x())/width(), gen->t.sample_end), 
								gen->t.loop_start
					);
					e->accept();
					engine::get()->deferred_commands.write(boost::bind(
						&waveform_widget::snap_to_zero, this, 
							gen->t.sample_start, gen->t.sample_end, gen->t.loop_start, loop_end));


				} else {
					double sample_end = std::max((double)(e->x())/width(), gen->t.sample_start);
					double loop_end = std::min(gen->t.sample_end, gen->t.loop_end);
					e->accept();
					engine::get()->deferred_commands.write(
						boost::bind(&waveform_widget::snap_to_zero, this, 
							gen->t.sample_start, sample_end, gen->t.loop_start, loop_end));
				}
			}
		}

		void mousePressEvent(QMouseEvent *e) {
			if (e->button() == Qt::LeftButton) {
				if (e->modifiers() & Qt::ShiftModifier) {
					double loop_start = std::min(std::max((double)(e->x())/width(), gen->t.sample_start), gen->t.sample_end);
					e->accept();
					engine::get()->deferred_commands.write(
						boost::bind(&waveform_widget::snap_to_zero, this, 
							gen->t.sample_start, gen->t.sample_end, loop_start, gen->t.loop_end));
				} else {
					double sample_start = std::min((double)(e->x())/width(), gen->t.sample_end);
					double loop_start = std::max(gen->t.sample_start, gen->t.loop_start);
					e->accept();
					engine::get()->deferred_commands.write(
						boost::bind(&waveform_widget::snap_to_zero, this, 
							sample_start, gen->t.sample_end, loop_start, gen->t.loop_end));
				}
			}
			if (e->button() == Qt::RightButton) {
				if (e->modifiers() & Qt::ShiftModifier) {
					double loop_end = std::max(std::min((double)(e->x())/width(), gen->t.sample_end), gen->t.sample_start);
					e->accept();
					engine::get()->deferred_commands.write(
						boost::bind(&waveform_widget::snap_to_zero, this, 
							gen->t.sample_start, gen->t.sample_end, gen->t.loop_start, loop_end));
				} else {
					double sample_end = std::max((double)(e->x())/width(), gen->t.sample_start);
					double loop_end = std::min(gen->t.sample_end, gen->t.loop_end);
					e->accept();
					engine::get()->deferred_commands.write(
						boost::bind(&waveform_widget::snap_to_zero, this, 
							gen->t.sample_start, sample_end, gen->t.loop_start, loop_end));
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
