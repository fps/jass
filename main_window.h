#ifndef JASS_MAIN_WINDOW_HH
#define JASS_MAIN_WINDOW_HH

#include <string>
#include <fstream>

#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QFileSystemModel>

#include "engine.h"
#include "assign.h"
#include "generator.h"
#include "jass.hxx"

class main_window : public QMainWindow {
	Q_OBJECT

	QFileSystemModel file_system_model;
	QTreeView file_system_view;

	//! This is the generated object representation of an xml document..
	Jass::Jass jass;

	engine &engine_;

	std::string setup_file_name;

	void print_foo() {
		std::cout << "fooooo" << std::endl;
	}

	public slots:

		void handle_jack_session_event(jack_session_event_t *ev) {
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

		void sample_double_clicked(const QModelIndex &index);

		void save_setup(const std::string &file_name) {
			try {
				std::ofstream f(file_name.c_str());
				Jass::Jass_(f, jass);
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

	
		void load_setup(const std::string &file_name) {
			try {
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
					engine_.commands.write(assign(engine_.gens->t[i++], p));
				}
				jass = jass_;
				setup_file_name = file_name;
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

	public:
		main_window(engine &e) :
			engine_(e)
		{
			setWindowTitle("jass - jack simple sampler");

			//engine_.commands.write(boost::bind(&generator::set_sample, engine_.gens->t[0]->t, disposable_sample::create(sample("foo"))));

			connect(
				&file_system_view, 
				SIGNAL(doubleClicked(const QModelIndex&)), 
				this, 
				SLOT(sample_double_clicked(const QModelIndex&)),
				Qt::QueuedConnection
			);

			file_system_model.setRootPath("/");
			file_system_view.setModel(&file_system_model);
			//file_system_view.setExpanded(file_system_model.index("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples.old/DrumKits/Club basic/"), true);
			//file_system_view.scrollTo(file_system_model.index("/media/b74d014a-92ba-4835-b559-64a7bd913819/Samples.old/DrumKits/Club basic/"));
			setCentralWidget(&file_system_view);
		}
};

#endif
