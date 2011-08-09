#ifndef JASS_MAIN_WINDOW_HH
#define JASS_MAIN_WINDOW_HH

#include <string>
#include <fstream>
#include <cstdlib>
#include <iterator>

#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QTreeView>
#include <QTableWidget>
#include <QFileSystemModel>
#include <QApplication>
#include <QHeaderView>
#include <QDockWidget>
#include <QSpinBox>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>

#include "engine.h"
#include "assign.h"
#include "generator.h"
#include "jass.hxx"

class main_window : public QMainWindow {
	Q_OBJECT

	command_ringbuffer deferred_gui_commands;

	QFileSystemModel file_system_model;
	QTreeView *file_system_view;
	bool file_clicked;

	QTableWidget *generator_table;

	QDockWidget *file_system_view_dock_widget;

	engine &engine_;

	public:
		std::string setup_file_name;

	public slots:
		void sample_file_clicked(const QModelIndex &index) {
			file_clicked = true;

			if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier))
				return;

			try {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						std::string(file_system_model.fileName(index).toLatin1()),
						disposable_sample::create(
							sample(std::string(file_system_model.filePath(index).toLatin1()))
						)
					)
				);
				write_blocking_command(assign(engine_.auditor_gen, p));
				write_blocking_command(boost::bind(&engine::play_auditor, boost::ref(engine_)));
			} catch (...) {
				std::cout << "something went wrong" << std::endl;
			}

		}

		void sample_file_double_clicked(const QModelIndex &index) {
			file_clicked = true;

			try {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						std::string(file_system_model.fileName(index).toLatin1()),
						disposable_sample::create(
							sample(std::string(file_system_model.filePath(index).toLatin1()))
						)
					)
				);
				std::cout << "writing command" << std::endl;
				disposable_generator_list_ptr l = disposable_generator_list::create(engine_.gens->t);
				l->t.push_back(p);
				write_blocking_command(assign(engine_.gens, l));
				deferred_gui_commands.write(boost::bind(&main_window::update_generator_table, this));
			} catch (...) {
				std::cout << "something went wrong" << std::endl;
			}
		}
		
		void generator_property_changed(void) {
			std::cout << "property" << std::endl;
			unsigned int row = generator_table->currentRow();
			generator_list::iterator i = engine_.gens->t.begin();
			std::advance(i, row);
			write_command(assign((*i)->t.name, std::string(((generator_table->item(row, 0))->text().toLatin1()))));
			write_command(assign((*i)->t.channel, (((QSpinBox*)generator_table->cellWidget(row, 3))->value())));
			write_command(assign((*i)->t.note, (((QSpinBox*)generator_table->cellWidget(row, 4))->value())));
			write_command(assign((*i)->t.min_note, (((QSpinBox*)generator_table->cellWidget(row, 5))->value())));
			write_command(assign((*i)->t.max_note, (((QSpinBox*)generator_table->cellWidget(row, 6))->value())));
			write_command(assign((*i)->t.min_velocity, (((QSpinBox*)generator_table->cellWidget(row, 7))->value())));
			write_command(assign((*i)->t.max_velocity, (((QSpinBox*)generator_table->cellWidget(row, 8))->value())));
			write_command(assign((*i)->t.velocity_factor, (((QSlider*)generator_table->cellWidget(row, 9))->value())));
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
			if (engine_.commands.can_write()) {
				engine_.commands.write(f);
			}
		}
		
		void write_blocking_command(boost::function<void(void)> f) {
			//! Will be reenabled by acknowledgement 
			if (engine_.commands.can_write()) {
				setEnabled(false);
				engine_.commands.write(f);
			}
		}

		void save_setup(const std::string &file_name) {
			try {
				std::ofstream f(file_name.c_str());
				Jass::Jass j;
				for(generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) 
					j.Generator().push_back(Jass::Generator(
						(*it)->t.name,
						(*it)->t.get_sample()->t.file_name,
						(*it)->t.voices->t.size(),
						(*it)->t.channel,
						(*it)->t.note,
						(*it)->t.min_note,
						(*it)->t.max_note,
						(*it)->t.min_velocity,
						(*it)->t.max_velocity,
						(*it)->t.velocity_factor
					));
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

		//! This should only be called by deferred_gui_commands.read()()
		void update_generator_table() {
			generator_table->setRowCount(engine_.gens->t.size());

			int row = 0;
			for (generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) {
				generator_table->setItem(row, 0, new QTableWidgetItem((*it)->t.name.c_str()));
				generator_table->setItem(row, 1, new QTableWidgetItem((*it)->t.get_sample()->t.file_name.c_str()));

				generator_table->setCellWidget(row, 2, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,2))->setRange(0,127);
				((QSpinBox*)generator_table->cellWidget(row,2))->setValue((*it)->t.voices->t.size());
				connect(generator_table->cellWidget(row, 2), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 3, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,3))->setRange(0,16);
				((QSpinBox*)generator_table->cellWidget(row,3))->setValue((*it)->t.channel);
				connect(generator_table->cellWidget(row, 3), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 4, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,4))->setRange(-128,127);
				((QSpinBox*)generator_table->cellWidget(row,4))->setValue((*it)->t.note);
				connect(generator_table->cellWidget(row, 4), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 5, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,5))->setRange(0,127);
				((QSpinBox*)generator_table->cellWidget(row,5))->setValue((*it)->t.min_note);
				connect(generator_table->cellWidget(row, 5), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 6, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,6))->setRange(0,127);
				((QSpinBox*)generator_table->cellWidget(row,6))->setValue((*it)->t.max_note);
				connect(generator_table->cellWidget(row, 6), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 7, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,7))->setRange(0,127);
				((QSpinBox*)generator_table->cellWidget(row,7))->setValue((*it)->t.min_velocity);
				connect(generator_table->cellWidget(row, 7), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 8, new QSpinBox());
				((QSpinBox*)generator_table->cellWidget(row,8))->setRange(0,127);
				((QSpinBox*)generator_table->cellWidget(row,8))->setValue((*it)->t.max_velocity);
				connect(generator_table->cellWidget(row, 8), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				generator_table->setCellWidget(row, 9, new QSlider(Qt::Horizontal));
				((QSlider*)generator_table->cellWidget(row,9))->setRange(-1.0, 1.0);
				((QSlider*)generator_table->cellWidget(row,9))->setValue((*it)->t.velocity_factor);

				connect(generator_table->cellWidget(row, 9), SIGNAL(valueChanged(int)), this, SLOT(generator_property_changed()));

				++row;
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
						generator(
							(*it).Name(),
							disposable_sample::create((*it).Sample()),
							(*it).Polyphony(),
							(*it).Channel(),
							(*it).Note(),
							(*it).MinNote(),
							(*it).MaxNote(),
							(*it).MinVelocity(),
							(*it).MaxVelocity(),
							(*it).VelocityFactor()
						));
					std::cout
						<< "gen args: "
						<< " " << p->t.name
						<< " " << p->t.voices->t.size()
						<< " " << p->t.channel
						<< " " << p->t.note
						<< " " << p->t.min_note
						<< " " << p->t.max_note
						<< " " << p->t.min_velocity
						<< " " << p->t.max_velocity
						<< " " << p->t.velocity_factor
						<< std::endl;

					l->t.push_back(p);
					std::cout 
						<< "gen args: " 
						<< " " << (*it).Name()
						<< " " << (*it).Polyphony()
						<< " " << (*it).Channel()
						<< " " << (*it).Note()
						<< " " << (*it).MinNote()
						<< " " << (*it).MaxNote()
						<< " " << (*it).MinVelocity()
						<< " " << (*it).MaxVelocity()
						<< " " << (*it).VelocityFactor()
						<< std::endl;
				}
				write_blocking_command(assign(engine_.gens, l));
				setup_file_name = file_name;
				deferred_gui_commands.write(boost::bind(&main_window::update_generator_table, this));
				//! Then write them in one go, replacing the whole gens collection
			} catch(...) {
				std::cout << "something went wrong loading some file" << std::endl;
			}
		}
	
		void check_acknowledgements() {
			if (engine_.acknowledgements.can_read()) {
				while(engine_.acknowledgements.can_read()) engine_.acknowledgements.read();
				while(deferred_gui_commands.can_read()) deferred_gui_commands.read()();
				setEnabled(true);
			}
		}

		void closeEvent(QCloseEvent *event)
		 {
			QSettings settings;
			settings.setValue("geometry", saveGeometry());
			settings.setValue("windowState", saveState());
			settings.setValue("fileSystemViewState", file_system_view->header()->saveState());
			if (file_clicked) {
				settings.setValue(
					"fileSystemLastFile", 
					file_system_model.filePath(file_system_view->currentIndex()));
			}

			QWidget::closeEvent(event);
		}

	public:
		bool eventFilter(QObject *obj, QEvent *ev) {
			std::cout << "filter" << std::endl;
			if (ev->type() == QEvent::Hide || ev->type() == QEvent::Close) {
				std::cout << "eat" << std::endl;
				return true;
			}
			return false;
		}

		main_window(engine &e) :
			engine_(e),
			file_clicked(false),
			deferred_gui_commands(1024)
		{
			setWindowTitle("jass - jack simple sampler");

			QMenuBar *menu_bar = new QMenuBar();				
				QMenu *file_menu = new QMenu("&File");
				menu_bar->addMenu(file_menu);
					file_menu->addAction("&Open...");
					file_menu->addSeparator();
					file_menu->addAction("&Save");
					file_menu->addAction("Save &As...");
					file_menu->addSeparator();
					file_menu->addAction("&Quit");
				QMenu *generator_menu = new QMenu("&Generator");
				menu_bar->addMenu(generator_menu);
					generator_menu->addAction("&Remove");
					generator_menu->addSeparator();
					generator_menu->addAction("&New from File Dialog...");
				QMenu *help_menu = new QMenu("&Help");
					help_menu->addAction("&Help");
					help_menu->addAction("&About");
	
			setMenuBar(menu_bar);

			file_system_model.setRootPath("/");
			file_system_view = new QTreeView();
			file_system_view->setModel(&file_system_model);

			connect(
				file_system_view, 
				SIGNAL(doubleClicked(const QModelIndex&)), 
				this, 
				SLOT(sample_file_double_clicked(const QModelIndex&)),
				Qt::QueuedConnection
			);

			connect(
				file_system_view, 
				SIGNAL(clicked(const QModelIndex&)), 
				this, 
				SLOT(sample_file_clicked(const QModelIndex&)),
				Qt::QueuedConnection
			);

			generator_table = new QTableWidget();

			generator_table->setColumnCount(10);
			QStringList headers;
			headers 
				<< "Name"
				<< "Sample" 
				<< "Poly."
				<< "Ch." 
				<< "Note."
				<< "Min. Note" 
				<< "Max. Note" 
				<< "Min. Vel." 
				<< "Max. Vel." 
				<< "Vel. Factor";

			generator_table->setHorizontalHeaderLabels(headers);
			setCentralWidget(generator_table);

			file_system_view_dock_widget = new QDockWidget();
			file_system_view_dock_widget->setObjectName("FileSystemDockWidget");
			file_system_view_dock_widget->setWidget(file_system_view);

			addDockWidget(Qt::LeftDockWidgetArea, file_system_view_dock_widget);

			QDockWidget *file_dialog_dock_widget = new QDockWidget();
			QFileDialog *file_dialog = new QFileDialog(this, Qt::SubWindow);//"Select a Sample", "/", "*");
			connect(file_dialog, SIGNAL(finished(int)), file_dialog, SLOT(open()));
			//file_dialog->installEventFilter(this);

			file_dialog_dock_widget->setWidget(file_dialog);
			//addDockWidget(Qt::RightDockWidgetArea, file_dialog_dock_widget);


			QSettings settings;
			restoreGeometry(settings.value("geometry").toByteArray());
			restoreState(settings.value("windowState").toByteArray());
			file_system_view->header()->restoreState(settings.value("fileSystemViewState").toByteArray());
			file_system_view->expand(file_system_model.index(settings.value("fileSystemLastFile").toString()));
			file_system_view->scrollTo(file_system_model.index(settings.value("fileSystemLastFile").toString()));		}
};

#endif
