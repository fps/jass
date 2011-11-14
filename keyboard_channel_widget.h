#ifndef JASS_KEYBOARD_CHANNEL_WIDGET_HH
#define JASS_KEYBOARD_CHANNEL_WIDGET_HH

#include <QWidget>
#include <QSpinBox>
#include <QHBoxLayout>

#include "generator.h"
#include "engine.h"
#include "keyboard_widget.h"

struct keyboard_channel_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	QSpinBox *channel_spin;

	public slots:
		void channel_changed(int channel) {
			engine::get()->write_command(assign(gen->t.channel, channel));
			engine::get()->deferred_commands.write(boost::bind(&keyboard_channel_widget::update, this));
		}

	public:
		keyboard_channel_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			QWidget(parent),
			gen(gen) 
		{
			QHBoxLayout *layout = new QHBoxLayout();
			channel_spin = new QSpinBox();
			channel_spin->setMinimum(0);
			channel_spin->setMaximum(15);
			channel_spin->setValue(gen->t.channel);
			layout->addWidget(channel_spin, 0);
			connect(channel_spin, SIGNAL(valueChanged(int)), this, SLOT(channel_changed(int)));
			layout->addWidget(new keyboard_widget(gen), 1);
			setLayout(layout);
		}
};

#endif
