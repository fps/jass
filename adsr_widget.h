#ifndef JASS_ADSR_WIDGET_HH
#define JASS_ADSR_WIDGET_HH

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
#include <QSlider>
#include <QDial>
#include <QGridLayout>

#include <cmath>

#include "generator.h"
#include "dial_widget.h"
#include "engine.h"
#include "assign.h"

struct adsr_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	dial_widget *gain;
	dial_widget *a;
	dial_widget *d;
	dial_widget *s;
	dial_widget *r;
		
	public slots:
		void changed(double) {
			engine::get()->write_command(assign(gen->t.gain, gain->value()));
			engine::get()->write_command(assign(gen->t.attack_g, a->value()));
			engine::get()->write_command(assign(gen->t.decay_g, d->value()));
			engine::get()->write_command(assign(gen->t.sustain_g, s->value()));
			engine::get()->write_command(assign(gen->t.release_g, r->value()));
			engine::get()->deferred_commands.write(boost::bind(&adsr_widget::update, this));
		}


	public:
		adsr_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			QWidget(parent),
			gen(gen)
		{

			gain = new dial_widget();
			gain->setToolTip("Gain");
			gain->set_value(gen->t.gain);
			gain->set_min_value(0);
			gain->set_max_value(2);
			connect(gain, SIGNAL(valueChanged(double)), this, SLOT(changed(double)));

			a = new dial_widget();
			a->setToolTip("Attack");
			a->set_value(gen->t.attack_g);
			a->set_min_value(0.001);
			a->set_max_value(3);
			connect(a, SIGNAL(valueChanged(double)), this, SLOT(changed(double)));

			d = new dial_widget();
			d->setToolTip("Decay");
			d->set_value(gen->t.decay_g);
			d->set_min_value(0.001);
			d->set_max_value(3);
			connect(d, SIGNAL(valueChanged(double)), this, SLOT(changed(double)));

			s = new dial_widget();
			s->setToolTip("Sustain");
			s->set_value(gen->t.sustain_g);
			s->set_min_value(0);
			s->set_max_value(1);
			connect(s, SIGNAL(valueChanged(double)), this, SLOT(changed(double)));

			r = new dial_widget();
			r->setToolTip("Release");
			r->set_value(gen->t.release_g);
			r->set_min_value(0.001);
			r->set_max_value(10);
			connect(r, SIGNAL(valueChanged(double)), this, SLOT(changed(double)));

			QGridLayout *layout = new QGridLayout();
			layout->addWidget(gain, 0, 0);
			layout->addWidget(a, 0, 1);
			layout->addWidget(d, 0, 2);
			layout->addWidget(s, 0, 3);
			layout->addWidget(r, 0, 4);
			setLayout(layout);
#if 0
			QPalette palette = QWidget::palette();
			palette.setColor(backgroundRole(), Qt::white);
			setPalette(palette);
#endif
		}

#if 0
		void paintEvent(QPaintEvent *)
		{
			QPainter painter(this);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.setPen(Qt::blue);
			painter.setBrush(Qt::blue);

			double total_length = gen->t.attack_g + gen->t.decay_g + 1.0 + gen->t.release_g;
			double stretch = width() / total_length;

			unsigned int n = 500;
			QVector<QPointF> points;
			points.push_back(QPointF(
				0.0, 
				height()-1
			));

			points.push_back(QPointF(
				gen->t.attack_g * stretch, 
				0.0
			));

			points.push_back(QPointF(
				(gen->t.attack_g + gen->t.decay_g) * stretch, 
				height() * (1.0 - gen->t.sustain_g)
			));

			points.push_back(QPointF(
				(gen->t.attack_g + gen->t.decay_g + 1.0) * stretch, 
				height() * (1.0 - gen->t.sustain_g)
			));

			points.push_back(QPointF(
				width(), 
				height()-1.0
			));

			painter.drawPolygon(&points[0], points.size(), Qt::OddEvenFill);

			painter.setPen(Qt::black);
			//painter.setCompositionMode(QPainter::CompositionMode_Difference);
			painter.drawLine(0, height() * (1.0 - gen->t.sustain_g), width(), height() * (1.0 - gen->t.sustain_g));
			painter.drawLine(stretch * gen->t.attack_g, 0, stretch * gen->t.attack_g, height());
			painter.drawLine(stretch * (gen->t.attack_g + gen->t.decay_g), 0, stretch * (gen->t.attack_g + gen->t.decay_g), height());
			painter.drawLine(stretch * (gen->t.attack_g + gen->t.decay_g + 1.0), 0, stretch * (gen->t.attack_g + gen->t.decay_g + 1.0), height());
		}


		QSize minimumSizeHint() const
		{
			return QSize(50, 100);
		}

		QSize sizeHint() const
		{
			return QSize(200, 100);
		}
#endif
};

#endif
