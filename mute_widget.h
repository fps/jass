#ifndef JASS_MUTE_WIDGET_HH
#define JASS_MUTE_WIDGET_HH

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>

#include "generator.h"
#include "engine.h"

struct mute_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	QCheckBox *check_box;

	public slots:
		void checked(bool checked) {
			engine::get()->write_command(assign(gen->t.muted, checked));
		}

	public:
		mute_widget(disposable_generator_ptr g, QWidget *parent = 0) :
			QWidget(parent),
			gen(g)
		{
			QHBoxLayout *layout = new QHBoxLayout();
			check_box = new QCheckBox();

			connect(check_box, SIGNAL(clicked(bool)), this, SLOT(checked(bool)));

			layout->addWidget(check_box);
			setLayout(layout);

			setToolTip("Toggle to mute");
		}
};

#endif
