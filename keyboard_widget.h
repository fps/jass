#ifndef JASS_KEYBOARD_WIDGET_HH
#define JASS_KEYBOARD_WIDGET_HH

#include <QWidget>
#include <QPainter>

#include "generator.h"

struct keyboard_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	public:
		keyboard_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
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

			for (unsigned int i = 0; i < 128; ++i) {
				if (
					i % 12 ==  1 ||
					i % 12 ==  3 ||
					i % 12 ==  6 ||
					i % 12 ==  8 ||
					i % 12 == 10
				) {
					painter.setPen(Qt::black);
					painter.setBrush(Qt::black);
				} else {
					painter.setPen(Qt::white);
					painter.setBrush(Qt::white);
				}

				painter.drawRect(width() * (double(i)/128), 0, width()/(double)128, 3.0 * height() / 4.0);

				if (gen->t.min_note <= i && i <= gen->t.max_note) {
					painter.setPen(Qt::gray);
					painter.setBrush(Qt::gray);
					painter.drawRect(width() * (double(i)/128), 3.0*height()/4.0, width()/(double)128, height()/4.0);
				}

				if (gen->t.note == i) {
					painter.setPen(Qt::red);
					painter.setBrush(Qt::red);
					painter.drawRect(width() * (double(i)/128), 3.0*height()/4.0, width()/(double)128, height()/4.0);
				}

				painter.setPen(Qt::black);
				painter.setBrush(Qt::black);
				painter.drawLine(width() * double(i)/128, 0, width() * double(i)/128, height()); 
			}

		}

		QSize minimumSizeHint() const
		{
			return QSize(200, 10);
		}

		QSize sizeHint() const
		{
			return QSize(500, 10);
		}
};

#endif