#ifndef JASS_VELOCITY_WIDGET_HH
#define JASS_VELOCITY_WIDGET_HH

#include <QWidget>
#include <QHBoxLayout>


#include "generator.h"
#include "velocity_range_widget.h"
#include "dial_widget.h"
#include "engine.h"
#include "assign.h"

struct velocity_widget : public QWidget {
	Q_OBJECT

	dial_widget *factor;
#if 0
	velocity_range_widget *range;
#endif

	disposable_generator_ptr gen;

	public slots:
		void factor_changed(double v) {
			engine::get()->write_command(assign(gen->t.velocity_factor, v));
			engine::get()->deferred_commands.write(boost::bind(&velocity_widget::update, this));
		}

	public:
		velocity_widget(disposable_generator_ptr g, QWidget *parent = 0) :
			QWidget(parent),
			gen(g)
		{
			QHBoxLayout *layout = new QHBoxLayout();
			factor = new dial_widget();
			factor->set_min_value(0);
			factor->set_max_value(2);
			factor->set_value(gen->t.velocity_factor);
			factor->setToolTip("Factor");
			connect(factor, SIGNAL(valueChanged(double)), this, SLOT(factor_changed(double)));
			layout->addWidget(factor, 0);
			layout->addWidget(new velocity_range_widget(gen), 1);
			setLayout(layout);
		}	
};

#endif

