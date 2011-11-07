#ifndef JASS_GENERATOR_WIDGET_HH
#define JASS_GENERATOR_WIDGET_HH

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>

#include "generator.h"
#include "waveform_widget.h"
#include "adsr_widget.h"

struct generator_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	QLineEdit *name;
	QLineEdit *sample;

	waveform_widget *waveform;

	public:
		generator_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			gen(gen),
			QWidget(parent) 
		{
			QGridLayout *layout = new QGridLayout();

			QGridLayout *sample_layout = new QGridLayout();

			int col = 0;
			sample_layout->addWidget(new QLabel("Name"), 0, 0);
			name = new QLineEdit(gen->t.name.c_str());
			sample_layout->addWidget(name, 1, 0);

			sample_layout->addWidget(new QLabel("Sample"), 2, 0);
			sample = new QLineEdit(gen->t.sample_->t.file_name.c_str());
			sample_layout->addWidget(sample, 3, 0);

			layout->addLayout(sample_layout, 0, col++);

			waveform = new waveform_widget(gen);
			layout->addWidget(waveform, 0, col++);

			setLayout(layout);
			
			//show();
		}
};

#endif
