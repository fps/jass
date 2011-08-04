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

	public slots:
		void sample_double_clicked(const QModelIndex &index) {
			try {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						disposable_sample::create(
							sample(std::string(file_system_model.filePath(index).toLatin1()))
						)
					)
				);

				std::cout << "writing command" << std::endl;
				engine_.commands.write(assign(engine_.gens->t[0], p));

			} catch (...) {
				std::cout << "something went wrong" << std::endl;
			}
		}

	public:
		main_window(engine &e) :
			engine_(e)
		{

			#if 0
			for (int i = 0; i < 2; ++i) {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						disposable_sample::create(
							sample("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples/AdamKeshen44/AKALOOP3.wav")
						)
					)
				);

				engine_.commands.write(assign(engine_.gens->t[i], p));
				sleep(2);
			}
			#endif

			setWindowTitle("jass - jack simple sampler");

			connect(
				&file_system_view, 
				SIGNAL(doubleClicked(const QModelIndex&)), 
				this, 
				SLOT(sample_double_clicked(const QModelIndex&))
			);

			file_system_model.setRootPath("/");
			file_system_view.setModel(&file_system_model);
			file_system_view.setExpanded(file_system_model.index("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples.old/DrumKits/Club basic/"), true);
			file_system_view.scrollTo(file_system_model.index("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples.old/DrumKits/Club basic/"));
			setCentralWidget(&file_system_view);
		}
};

#endif
