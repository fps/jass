#ifndef JASS_MAIN_WINDOW_HH
#define JASS_MAIN_WINDOW_HH

#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <iterator>

#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QTreeView>
#include <QTableWidget>
#include <QFileSystemModel>
#include <QComboBox>
#include <QApplication>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QDockWidget>
#include <QSpinBox>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QTextEdit>
#include <QDial>

#include "jass.hxx"

#include "engine.h"
#include "assign.h"
#include "generator.h"
#include "generator_widget.h"
#include "keyboard_channel_widget.h"
#include "velocity_widget.h"
#include "sample_range_widget.h"
#include "mute_widget.h"

class main_window : public QMainWindow {
	Q_OBJECT

	QTableWidget *generator_table;

	QFileDialog *file_dialog;
	QDockWidget *file_dialog_dock_widget;

	QTextEdit *log_text_edit;
	QDockWidget *log_text_edit_dock_widget;
	
	engine &engine_;

	public:
		std::string setup_file_name;

	public slots:
		void audit_sample_file(const QString &path) {
			if(!(QApplication::keyboardModifiers() & Qt::ControlModifier)) return;

			try {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						std::string(path.toLatin1()),
						disposable_sample::create(
							sample(std::string(path.toLatin1()), jack_get_sample_rate(engine_.jack_client))
						)
					)
				);
				setEnabled(false);
					engine_.write_command(assign(engine_.auditor_gen, p));
					engine_.write_command(boost::bind(&engine::play_auditor, boost::ref(engine_)));
				engine_.deferred_commands.write(boost::bind(&main_window::setEnabled, this, true));

				log_text_edit->append("Loaded audit sample: ");
				log_text_edit->append(path);
			} catch (...) {
				log_text_edit->append("Something went wrong auditing sample: ");
				log_text_edit->append(path);
				// std::cout << "something went wrong" << std::endl;
			}

		}

		//! Only allow disabling if the engine is still active
		void setEnabled(bool enable) {
			if (enable) QMainWindow::setEnabled(enable);

			if (!enable && engine_.active) QMainWindow::setEnabled(enable);
		}

		void load_sample_file() {
			if(QApplication::keyboardModifiers() & Qt::AltModifier) return;

			disposable_generator_list_ptr l = disposable_generator_list::create(engine_.gens->t);

			for (unsigned int index = 0; index < file_dialog->selectedFiles().size(); ++index) {
				try {
					disposable_generator_ptr p = disposable_generator::create(
						generator(
							std::string(QFileInfo(file_dialog->selectedFiles()[index]).baseName().toLatin1()),
							disposable_sample::create(
								sample(std::string(file_dialog->selectedFiles()[index].toLatin1()), jack_get_sample_rate(engine_.jack_client))
							)
						)
					);
					std::cout << "writing command" << std::endl;
					l->t.push_back(p);
					log_text_edit->append("Loaded sample: ");
					log_text_edit->append(file_dialog->selectedFiles()[index]);
				} catch (...) {
					log_text_edit->append("Something went wrong loading Sample:");
					log_text_edit->append(file_dialog->selectedFiles()[index]);
				}
			}
			setEnabled(false);
				engine_.write_command(assign(engine_.gens, l));
				engine_.deferred_commands.write(boost::bind(&main_window::update_generator_table, this));
			engine_.deferred_commands.write(boost::bind(&main_window::setEnabled, this, true));
		}
		
#ifndef NO_JACK_SESSION
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
#endif

		void save_setup(const std::string &file_name) {
			try {
				std::ofstream f(file_name.c_str());
				Jass::Jass j(engine_.voices->t.size());
				for(generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) {
					Jass::Generator jg((*it)->t.name, (*it)->t.sample_->t.file_name);
					jg.Name() = (*it)->t.name;
					jg.Sample() = (*it)->t.sample_->t.file_name;
					jg.SampleStart() = (*it)->t.sample_start;
					jg.SampleEnd() = (*it)->t.sample_end;
					jg.Looping() = (*it)->t.looping;
					jg.LoopStart() = (*it)->t.loop_start;
					jg.LoopEnd() = (*it)->t.loop_end;
					jg.Muted() = (*it)->t.muted;
					jg.Gain() = (*it)->t.gain;
					jg.Channel() = (*it)->t.channel;
					jg.Note() = (*it)->t.note;
					jg.MinNote() = (*it)->t.min_note;
					jg.MaxNote() = (*it)->t.max_note;
					jg.MinVelocity() = (*it)->t.min_velocity;
					jg.MaxVelocity() = (*it)->t.max_velocity;
					jg.VelocityFactor() = (*it)->t.velocity_factor;
					jg.AttackGain() = (*it)->t.attack_g;
					jg.DecayGain() = (*it)->t.decay_g;
					jg.SustainGain() = (*it)->t.sustain_g;
					jg.ReleaseGain() = (*it)->t.release_g;

#if 0
					j.Generator().push_back(Jass::Generator(
						(*it)->t.name,
						(*it)->t.sample_->t.file_name,
						(*it)->t.sample_start,
						(*it)->t.sample_end,
						(*it)->t.looping,
						(*it)->t.loop_start,
						(*it)->t.loop_end,
						(*it)->t.muted,
						(*it)->t.gain,
						(*it)->t.channel,
						(*it)->t.note,
						(*it)->t.min_note,
						(*it)->t.max_note,
						(*it)->t.min_velocity,
						(*it)->t.max_velocity,
						(*it)->t.velocity_factor,
						(*it)->t.attack_g,
						(*it)->t.decay_g,
						(*it)->t.sustain_g,
						(*it)->t.release_g
					));
#endif
					j.Generator().push_back(jg);
				}
				Jass::Jass_(f, j);
			} catch (...) {
				log_text_edit->append(("something went wrong saving the setup: " + file_name).c_str());
			}
		}

		void load_setup() {
			QString setup_file_name = QFileDialog::getOpenFileName();
			if (!setup_file_name.isNull())
				load_setup(std::string(setup_file_name.toLatin1()));
		}

		void save_setup_as() {
			QString setup_file_name = QFileDialog::getSaveFileName();
			if (!setup_file_name.isNull())
				save_setup(std::string(setup_file_name.toLatin1()));
		}

		void save_setup() {
			// std::cout << "save_setup" << std::endl;
			if (setup_file_name == "") {
				save_setup_as();
				return;
			}
			save_setup(setup_file_name);
		}
		
		//! This should only be called by deferred_gui_commands.read()()
		void update_generator_table() {
			//generator_table->setRowCount(0);
			generator_table->setRowCount(engine_.gens->t.size());

			int row = 0;
			for (generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) {
				int col = 0;
				generator_table->setCellWidget(row, col++, new mute_widget(*it));
				generator_table->setCellWidget(row, col++, new adsr_widget(*it));
				generator_table->setItem(row, col++, new QTableWidgetItem(QString((*it)->t.name.c_str())));
				generator_table->setCellWidget(row, col++, new keyboard_channel_widget((*it)));
				generator_table->setCellWidget(row, col++, new velocity_widget((*it)));
				generator_table->setCellWidget(row, col++, new sample_range_widget((*it)));
				generator_table->setItem(row, col++, new QTableWidgetItem(QString((*it)->t.sample_->t.file_name.c_str())));
				//generator_widget *w = new generator_widget(*it);
				//generator_table->setCellWidget(row++, 0, w);
				row++;
			}

			generator_table->resizeColumnsToContents();
			//generator_table->resizeRowsToContents();
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
				for(Jass::Jass::Generator_const_iterator it = jass_.Generator().begin(); it != jass_.Generator().end(); ++it) {
					log_text_edit->append(QString("Loading sample: %1").arg((*it).Sample().c_str()));
					disposable_generator_ptr p = disposable_generator::create(
						generator(
							(*it).Name(),
							disposable_sample::create(sample((*it).Sample(), jack_get_sample_rate(engine_.jack_client)))
						)
					);
					if ((*it).SampleStart()) p->t.sample_start = *(*it).SampleStart();
					if ((*it).SampleEnd()) p->t.sample_end = *(*it).SampleEnd();
					if ((*it).Looping()) p->t.looping = *(*it).Looping();
					if ((*it).LoopStart()) p->t.loop_start = *(*it).LoopStart();
					if ((*it).LoopEnd()) p->t.loop_end = *(*it).LoopEnd();
					if ((*it).Muted()) p->t.muted = *(*it).Muted();
					if ((*it).Gain()) p->t.gain = *(*it).Gain();
					if ((*it).Channel()) p->t.channel = *(*it).Channel();
					if ((*it).Note()) p->t.note = *(*it).Note();
					if ((*it).MinNote()) p->t.min_note = *(*it).MinNote();
					if ((*it).MaxNote()) p->t.max_note = *(*it).MaxNote();
					if ((*it).MinVelocity()) p->t.min_velocity = *(*it).MinVelocity();
					if ((*it).MaxVelocity()) p->t.max_velocity = *(*it).MaxVelocity();
					if ((*it).VelocityFactor()) p->t.velocity_factor = *(*it).VelocityFactor();
					if ((*it).AttackGain()) p->t.attack_g = *(*it).AttackGain();
					if ((*it).DecayGain()) p->t.decay_g = *(*it).DecayGain();
					if ((*it).SustainGain()) p->t.sustain_g = *(*it).SustainGain();
					if ((*it).ReleaseGain()) p->t.release_g = *(*it).ReleaseGain();

					l->t.push_back(p);
					log_text_edit->append(QString("Done loading sample: %1").arg((*it).Sample().c_str()));
					QApplication::processEvents();
				}

				setup_file_name = file_name;

				setEnabled(false);
					engine_.write_command(assign(engine_.gens, l));
					disposable_gvoice_vector_ptr voices(disposable_gvoice_vector::create(std::vector<gvoice>(jass_.Polyphony())));
					engine_.write_command(assign(engine_.voices, voices));
				engine_.deferred_commands.write(boost::bind(&main_window::update_generator_table, this));
				engine_.deferred_commands.write(boost::bind(&main_window::setEnabled, this, true));
				//! Then write them in one go, replacing the whole gens collection
			} catch(...) {
				log_text_edit->append(("something went wrong loading file: " + file_name + ". Try fixing your filesystem mounts, etc, then try reloading the setup").c_str());
			}
		}
	

		void closeEvent(QCloseEvent *event) {
			QSettings settings;
			settings.setValue("geometry", saveGeometry());
			settings.setValue("windowState", saveState());
			QWidget::closeEvent(event);
		}

		void remove_generator() {
			if (generator_table->currentRow() >= 0 && generator_table->currentRow() < generator_table->rowCount()) {
				// std::cout << "current row: " << generator_table->currentRow() << std::endl;
				disposable_generator_list_ptr l = disposable_generator_list::create(engine_.gens->t);
				generator_list::iterator it = l->t.begin();
				std::advance(it, generator_table->currentRow());
				l->t.erase(it);
				setEnabled(false);
					engine_.write_command(assign(engine_.gens, l));
					engine_.deferred_commands.write(boost::bind(&main_window::update_generator_table, this));
				engine_.deferred_commands.write(boost::bind(&main_window::setEnabled, this, true));
			}
		}

		void show_about_text() {
			log_text_edit->append("-------------------");
			log_text_edit->append("This is jass - Jack Simple Sampler");
			log_text_edit->append("License: Gnu General Public License v3.0 or later");
			log_text_edit->append("Author: Florian Paul Schmidt (mista.tapas at gmx.net)");
			log_text_edit->append("Thanks to the #lad inhabitants on irc.freenode.org, especially nedko, thorbenh, thorwil and others i forgot :D");
			log_text_edit->append("-------------------");
		}

		void show_help_text() {
			log_text_edit->append("-------------------");
			log_text_edit->append("Quick Tutorial:");
			log_text_edit->append("Take note of the tooltips. They should explain ALL THE THINGS!!!");
			log_text_edit->append("-------------------");
		}

		void show_keyboard(bool show) {
			generator_table->setColumnHidden(3, !show);

		}


		void show_waveform(bool show) {
			generator_table->setColumnHidden(5, !show);

		}

	public:

		main_window(engine &e) :
			engine_(e)
		{
			setWindowTitle("jass - jack simple sampler");

			generator_table = new QTableWidget();
			QStringList headers;
			headers 
				<< "Mute"
				<< "Gain/ADSR"
				<< "Name"
				<< "Note-Range"
				<< "Velocity Factor/Range"
				<< "Waveform/Looping/Ranges"
				<< "Sample";

			generator_table->setColumnCount(headers.size());
			generator_table->setHorizontalHeaderLabels(headers);
			generator_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); 
			generator_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel); 
			connect(generator_table, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(generator_item_changed(QTableWidgetItem*)));
			setCentralWidget(generator_table);

			file_dialog_dock_widget = new QDockWidget();
			file_dialog_dock_widget->setObjectName("FileDialogDockWidget");
			file_dialog = new QFileDialog(this, Qt::SubWindow);
			file_dialog->setOption(QFileDialog::DontUseNativeDialog);
			file_dialog->setFileMode(QFileDialog::ExistingFile);
			
			connect(file_dialog, SIGNAL(finished(int)), file_dialog, SLOT(open()));
			connect(file_dialog, SIGNAL(finished(int)), this, SLOT(load_sample_file()));
			connect(file_dialog, SIGNAL(currentChanged(const QString&)), this, SLOT(audit_sample_file(const QString&)));

			file_dialog_dock_widget->setWidget(file_dialog);
			addDockWidget(Qt::LeftDockWidgetArea, file_dialog_dock_widget);

			log_text_edit = new QTextEdit();
			log_text_edit->setReadOnly(true);
			log_text_edit->append("Motivational Message: Everything will be allright... Now get to making music, you sucker!!!\n");
			log_text_edit->document()->setMaximumBlockCount(1000);
			log_text_edit_dock_widget = new QDockWidget();
			log_text_edit_dock_widget->setWidget(log_text_edit);
			log_text_edit_dock_widget->setObjectName("LogDockWidget");
			addDockWidget(Qt::BottomDockWidgetArea, log_text_edit_dock_widget);
			

			QSettings settings;
			restoreGeometry(settings.value("geometry").toByteArray());
			restoreState(settings.value("windowState").toByteArray());
			
			show_about_text();

			QMenuBar *menu_bar = new QMenuBar();				

				QMenu *file_menu = new QMenu("&File");
				menu_bar->addMenu(file_menu);
					connect(file_menu->addAction("&Open..."), SIGNAL(triggered(bool)), this, SLOT(load_setup()));
					file_menu->addSeparator();
					QAction *save_action = file_menu->addAction("&Save");
					save_action->setShortcut(QString("Ctrl+S"));
					save_action->setShortcutContext(Qt::ApplicationShortcut);

					connect(save_action, SIGNAL(triggered(bool)), this, SLOT(save_setup()));

					connect(file_menu->addAction("Save &As..."), SIGNAL(triggered(bool)), this, SLOT(save_setup_as()));
					file_menu->addSeparator();

					connect(file_menu->addAction("&Quit"), SIGNAL(triggered(bool)), this, SLOT(close()));

				QMenu *generator_menu = new QMenu("&Generator");
				menu_bar->addMenu(generator_menu);
					connect(generator_menu->addAction("&Remove"), SIGNAL(triggered(bool)), this, SLOT(remove_generator()));;


				QMenu *view_menu = new QMenu("&View");
				menu_bar->addMenu(view_menu);
					QAction *show_keyboard_action = new QAction("Show &Note Range", 0);
						show_keyboard_action->setCheckable(true);
						view_menu->addAction(show_keyboard_action);
						connect(show_keyboard_action, SIGNAL(toggled(bool)), this, SLOT(show_keyboard(bool)));
						show_keyboard_action->setChecked(true);
					QAction *show_waveform_action = new QAction("Show &Wavevorm", 0);
						show_waveform_action->setCheckable(true);
						view_menu->addAction(show_waveform_action);
						connect(show_waveform_action, SIGNAL(toggled(bool)), this, SLOT(show_waveform(bool)));
						show_waveform_action->setChecked(true);

				QMenu *help_menu = new QMenu("&Help");
				menu_bar->addMenu(help_menu);
					connect(help_menu->addAction("&Help in Log"), SIGNAL(triggered(bool)), this, SLOT(show_help_text()));
					connect(help_menu->addAction("&About"), SIGNAL(triggered(bool)), this, SLOT(show_about_text()));
	
			setMenuBar(menu_bar);

		}
};

#endif
