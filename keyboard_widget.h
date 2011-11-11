#ifndef JASS_KEYBOARD_WIDGET_HH
#define JASS_KEYBOARD_WIDGET_HH

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

#include <iostream>

#include "generator.h"
#include "engine.h"

struct keyboard_widget : public QWidget {
	Q_OBJECT

	disposable_generator_ptr gen;

	public:
		keyboard_widget(disposable_generator_ptr gen, QWidget *parent = 0) :
			QWidget(parent),
			gen(gen)
		{
			QPalette palette = QWidget::palette();
			//palette.setColor(backgroundRole(), Qt::white);
			setPalette(palette);
			setToolTip("Doubleclick to set Note/Min/Max. Left-click to set Note. Shift-left-click and drag to the right to sweep Note-Min to Note-Max");
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
					painter.drawRect(width() * (double(i)/128), 0, width()/(double)128 - 1, 3.0*height()/4.0);
				} 

				if (
					i % 12 == 5 ||
					i % 12 == 0 
				) {
					painter.setPen(Qt::black);
					painter.setBrush(Qt::black);
					painter.drawLine(width() * double(i)/128 - 1, 0, width() * double(i)/128 - 1, 3.0 * height()/4.0); 
				}
	
				if (gen->t.note == i) {
					painter.setPen(QColor(255, 0, 0, 63));
					painter.setBrush(QColor(255, 0, 0, 63));
					painter.drawRect(width() * (double(i)/128), 0, width()/(double)128 - 1, height());
				}

			}

			painter.setPen(QColor(0, 0, 255, 63));
			painter.setBrush(QColor(0, 0, 255, 63));
			painter.drawRect(width() * (double(gen->t.min_note)/128), 0, width() * (gen->t.max_note - gen->t.min_note)/(double)128 - 1, height());

		}

		QSize minimumSizeHint() const
		{
			return QSize(200, 10);
		}

		QSize sizeHint() const
		{
			return QSize(800, 10);
		}

		int start_move;

		void mousePressEvent(QMouseEvent *e) {
			if (e->button() == Qt::LeftButton) {
				if (e->modifiers() & Qt::ShiftModifier) 
					gen->t.min_note = (double)(e->x())/width() * 128;
				else {
					gen->t.note = (double)(e->x())/width() * 128;
				}
			}
			update();
			e->accept();
		}

		void mouseMoveEvent(QMouseEvent *e) {
			if ((e->buttons() & Qt::LeftButton) && (e->modifiers() & Qt::ShiftModifier)) {
				gen->t.max_note = (double)(e->x())/width() * 128;
			}
			update();
			e->accept();
		}

		void mouseReleaseEvent(QMouseEvent *e) {
			if (e->modifiers() & Qt::ShiftModifier) {
				e->accept();
				return;
			}

			if (e->button() == Qt::LeftButton) {
				gen->t.note = (double)(e->x())/width() * 128;
				update();
			}
			update();
			e->accept();
		}

		void mouseDoubleClickEvent(QMouseEvent *e) {
			if (e->button() == Qt::LeftButton) {
				gen->t.note = gen->t.min_note = gen->t.max_note = (double)(e->x())/width() * 128;
				update();
			}
			update();
			e->accept();
		}

		
};

#endif
