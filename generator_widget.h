#ifndef JASS_GENERATOR_WIDGET_HH
#define JASS_GENERATOR_WIDGET_HH

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>

#include "generator.h"

struct generator_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	public:
		generator_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			gen(gen),
			QWidget(parent) 
		{
			QGridLayout *layout = new QGridLayout();
			layout->addWidget(new QLabel("Name"), 0, 0);
			layout->addWidget(new QLineEdit(), 1, 0);

			setLayout(layout);
			//show();
		}
};

#endif
