#ifndef JASS_MAIN_WINDOW_HH
#define JASS_MAIN_WINDOW_HH

#include <string>

#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QFileSystemModel>

#include "engine.h"
#include "assign.h"
#include "generator.h"

class main_window : public QMainWindow {
	Q_OBJECT

	QFileSystemModel file_system_model;
	QTreeView file_system_view;

	engine &engine_;

	void print_foo() {
		std::cout << "fooooo" << std::endl;
	}

	public slots:
		void sample_double_clicked(const QModelIndex &index);

	public:
		void check_acknowledgements() {
			if (engine_.acknowledgements.can_read()) {
				while(engine_.acknowledgements.can_read()) engine_.acknowledgements.read();
			}
			setEnabled(true);
		}

		main_window(engine &e) :
			engine_(e)
		{
			setWindowTitle("jass - jack simple sampler");

			connect(
				&file_system_view, 
				SIGNAL(doubleClicked(const QModelIndex&)), 
				this, 
				SLOT(sample_double_clicked(const QModelIndex&)),
				Qt::QueuedConnection
			);

			file_system_model.setRootPath("/");
			file_system_view.setModel(&file_system_model);
			file_system_view.setExpanded(file_system_model.index("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples.old/DrumKits/Club basic/"), true);
			file_system_view.scrollTo(file_system_model.index("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples.old/DrumKits/Club basic/"));
			setCentralWidget(&file_system_view);
		}
};

#endif
