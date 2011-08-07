#ifndef JASS_MAIN_WINDOW_HH
#define JASS_MAIN_WINDOW_HH

#include <string>
#include <fstream>
#include <cstdlib>

#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QTreeView>
#include <QTableWidget>
#include <QFileSystemModel>
#include <QDockWidget>

#include "engine.h"
#include "assign.h"
#include "generator.h"
#include "jass.hxx"

class main_window : public QMainWindow {
	Q_OBJECT

	QFileSystemModel file_system_model;
	QTreeView *file_system_view;
	QTableWidget *generator_table;

	QDockWidget *file_system_view_dock_widget;


	engine &engine_;

	public:
		std::string setup_file_name;

	public slots:
		void sample_double_clicked(const QModelIndex &index) {
			//engine_.commands.write(boost::bind(&main_window::print_foo, this));
			try {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						disposable_sample::create(
							sample(std::string(file_system_model.filePath(index).toLatin1()))
						)
					)
				);
				if(engine_.commands.can_write()) {
					std::cout << "writing command" << std::endl;
					disposable_generator_list_ptr l = disposable_generator_list::create(engine_.gens->t);
					l->t.push_back(p);
					write_blocking_command(assign(engine_.gens, l));
					sleep(1);
					update_generator_table();	
				} 
				else {
					std::cout << "full" << std::endl; 
				}
			} catch (...) {
				std::cout << "something went wrong" << std::endl;
			}
		}
		
		void handle_jack_session_event(jack_session_event_t *ev) {
			//! Copy pasta slightly adapted from the jack session client walkthrough..
			jack_session_event_t *e = (jack_session_event_t *) ev;
			char filename[10000];
			char command[10000];
			
			snprintf(filename, sizeof(filename), "%ssetup.xml", e->session_dir);
			snprintf(command, sizeof(command), "jass -U %s ${SESSION_DIR}setup.xml", e->client_uuid);
			
			save_setup(filename);
			
			ev->command_line = strdup(command);
			jack_session_reply(engine_.jack_client, e);
			
			if (ev->type == JackSessionSaveAndQuit) {
				close();
			}
			
			jack_session_event_free( ev );
		}

		//! Write command without blocking the GUI
		void write_command(boost::function<void(void)> f) {
			engine_.commands.write(f);
		}
		
		void write_blocking_command(boost::function<void(void)> f) {
			//! Will be reenabled by acknowledgement 
			setEnabled(false);

			engine_.commands.write(f);
		}

		void save_setup(const std::string &file_name) {
			try {
				std::ofstream f(file_name.c_str());
				Jass::Jass j;
				for(generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) 
					j.Generator().push_back(Jass::Generator((*it)->t.get_sample()->t.file_name, 0));
				Jass::Jass_(f, j);
			} catch (...) {
				std::cout << "something went wrong saving the setup" << std::endl;
			}
		}

		void save_setup() {
			std::cout << "save_setup" << std::endl;
			if (setup_file_name == "") {
				std::cout << "Ask user for filename" << std::endl;
				return;
			}
			save_setup(setup_file_name);
		}

		void update_generator_table() {
			generator_table->setRowCount(engine_.gens->t.size());

			int row = 0;
			for (generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) {
				generator_table->setItem(row++, 0, new QTableWidgetItem((*it)->t.get_sample()->t.file_name.c_str()));
			}
		}
	
		void load_setup(const std::string &file_name) {
			if (getenv("LADISH_APP_NAME") != 0) {
				setup_file_name = file_name;
				//! Don't fail in this case..
			}
			try {
				//! First try loading all generators
				disposable_generator_list_ptr l = 
					disposable_generator_list::create(
						generator_list());

				xsd_error_handler h;
				std::auto_ptr<Jass::Jass> j = Jass::Jass_(file_name, h, xml_schema::flags::dont_validate);
				Jass::Jass jass_ = *j;
				Jass::Jass_(std::cout, jass_);
 				int i = 0;
				setEnabled(false);
				for(Jass::Jass::Generator_const_iterator it = jass_.Generator().begin(); it != jass_.Generator().end(); ++it) {
					std::cout << "loading sample " << (*it).Sample() << std::endl;
					disposable_generator_ptr p = disposable_generator::create(
						disposable_sample::create((*it).Sample()));

					l->t.push_back(p);
				}
				sleep(1);
				write_blocking_command(assign(engine_.gens, l));

				setup_file_name = file_name;
				sleep(1);
				update_generator_table();
				//! Then write them in one go, replacing the whole gens collection
			} catch(...) {
				std::cout << "something went wrong loading some file" << std::endl;
			}
		}
	
		void check_acknowledgements() {
			if (engine_.acknowledgements.can_read()) {
				while(engine_.acknowledgements.can_read()) engine_.acknowledgements.read();
				setEnabled(true);
			}
		}

		void closeEvent(QCloseEvent *event)
		 {
			QSettings settings;
			settings.setValue("geometry", saveGeometry());
			settings.setValue("windowState", saveState());
			QWidget::closeEvent(event);
		}

	public:
		main_window(engine &e) :
			engine_(e)
		{
			setWindowTitle("jass - jack simple sampler");

			//engine_.commands.write(boost::bind(&generator::set_sample, engine_.gens->t[0]->t, disposable_sample::create(sample("foo"))));

			file_system_model.setRootPath("/");
			file_system_view = new QTreeView();
			file_system_view->setModel(&file_system_model);

			connect(
				file_system_view, 
				SIGNAL(doubleClicked(const QModelIndex&)), 
				this, 
				SLOT(sample_double_clicked(const QModelIndex&)),
				Qt::QueuedConnection
			);

			generator_table = new QTableWidget();

			generator_table->setColumnCount(1);
			QStringList headers;
			headers << "Generator";

			generator_table->setHorizontalHeaderLabels(headers);
			setCentralWidget(generator_table);

			file_system_view_dock_widget = new QDockWidget();
			file_system_view_dock_widget->setWidget(file_system_view);

			addDockWidget(Qt::LeftDockWidgetArea, file_system_view_dock_widget);

			QSettings settings;
			restoreGeometry(settings.value("main_window/geometry").toByteArray());
			restoreState(settings.value("main_window/windowState").toByteArray());
		}
};

#endif
