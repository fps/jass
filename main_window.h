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

#include "engine.h"
#include "assign.h"
#include "generator.h"
#include "generator_widget.h"
#include "jass.hxx"

class main_window : public QMainWindow {
	Q_OBJECT

	command_ringbuffer deferred_gui_commands;

	QTableWidget *generator_table;

	QFileDialog *file_dialog;
	QDockWidget *file_dialog_dock_widget;

	QTextEdit *log_text_edit;
	QDockWidget *log_text_edit_dock_widget;
	
	engine &engine_;

	int outstanding_acks;

	public:
		std::string setup_file_name;

	public slots:
		void audit_sample_file() {
			if(!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) return;

			try {
				disposable_generator_ptr p = disposable_generator::create(
					generator(
						std::string(file_dialog->selectedFiles()[0].toLatin1()),
						disposable_sample::create(
							sample(std::string(file_dialog->selectedFiles()[0].toLatin1()), jack_get_sample_rate(engine_.jack_client))
						)
					)
				);
				write_blocking_command(assign(engine_.auditor_gen, p));
				write_blocking_command(boost::bind(&engine::play_auditor, boost::ref(engine_)));
				
				log_text_edit->append("Loaded audit sample: ");
				log_text_edit->append(file_dialog->selectedFiles()[0]);
			} catch (...) {
				std::cout << "something went wrong" << std::endl;
			}

		}

		void load_sample_file() {
			if(QApplication::keyboardModifiers() & Qt::ShiftModifier) return;

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
			write_blocking_command(assign(engine_.gens, l));
			deferred_gui_commands.write(boost::bind(&main_window::update_generator_table, this));
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
				++outstanding_acks;
				engine_.commands.write(f);
			}
		}
		
		void write_blocking_command(boost::function<void(void)> f) {
			//! Will be reenabled by acknowledgement 
			if (engine_.commands.can_write()) {
				++outstanding_acks;
				setEnabled(false);
				engine_.commands.write(f);
			}
		}

		void save_setup(const std::string &file_name) {
			try {
				std::ofstream f(file_name.c_str());
				Jass::Jass j(engine_.voices->t.size());
				for(generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) 
					j.Generator().push_back(Jass::Generator(
						(*it)->t.name,
						(*it)->t.sample_->t.file_name,
						(*it)->t.sample_start,
						(*it)->t.sample_end,
						(*it)->t.looping,
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
			std::cout << "save_setup" << std::endl;
			if (setup_file_name == "") {
				save_setup_as();
				return;
			}
			save_setup(setup_file_name);
		}
		
		void set_continous_notes() {
			//! Get selected items
			QList<QTableWidgetItem*> items = generator_table->selectedItems();
			if (items.size() < 2) return;

			//! Get note of first selected generator

			generator_list::iterator it = engine_.gens->t.begin();
			std::advance(it, items[0]->row());
			unsigned int note = (*it)->t.note;
			write_command(assign((*it)->t.min_note, note));
			write_command(assign((*it)->t.max_note, note));

			for (unsigned int i = 1; i < items.size(); ++i) {
				generator_list::iterator it = engine_.gens->t.begin();
				std::advance(it, items[i]->row());
				write_command(assign((*it)->t.note, ++note));
				write_command(assign((*it)->t.min_note, note));
				write_command(assign((*it)->t.max_note, note));
			}
			deferred_gui_commands.write(boost::bind(&main_window::update_generator_table, this));
		}

		//! This should only be called by deferred_gui_commands.read()()
		void update_generator_table() {
			//generator_table->setRowCount(0);
			generator_table->setRowCount(engine_.gens->t.size());

			int row = 0;
			for (generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) {
				generator_widget *w = new generator_widget(*it);
				generator_table->setCellWidget(row++, 0, w);
			}

			generator_table->resizeColumnsToContents();
			generator_table->resizeRowsToContents();
#if 0
			int row = 0;
			for (generator_list::iterator it = engine_.gens->t.begin(); it != engine_.gens->t.end(); ++it) {
				int col = 0;
				QComboBox *combo_box;
				QDoubleSpinBox *double_spin_box;
				QSpinBox *spin_box;
				QCheckBox *check_box;

				//! Name
				generator_table->setItem(row, col++, new QTableWidgetItem((*it)->t.name.c_str()));

				//! Sample
				generator_table->setItem(row, col++, new QTableWidgetItem((*it)->t.sample_->t.file_name.c_str()));

				//! Start
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				double_spin_box->setDecimals(6);
				double_spin_box->setEnabled(false);
				double_spin_box->setRange(0.0, 100000.0); double_spin_box->setValue((*it)->t.sample_start);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);

				//! End
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				double_spin_box->setEnabled(false);
				double_spin_box->setDecimals(6);
				double_spin_box->setRange(0.0,100000.0); double_spin_box->setValue((*it)->t.sample_end);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);

				//! Looping
				check_box = new QCheckBox(); check_box->setProperty("row", row);
				check_box->setEnabled(false);
				check_box->setChecked((*it)->t.looping);
				connect(
					check_box, SIGNAL(stateChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, check_box);


				//! Gain
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				double_spin_box->setDecimals(6);
				double_spin_box->setMinimum(0.0); double_spin_box->setMaximum(100000.0); double_spin_box->setValue((*it)->t.gain);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);


				//! Channel
				spin_box = new QSpinBox(); spin_box->setProperty("row", row);
				spin_box->setRange(0,15); spin_box->setValue((*it)->t.channel);
				connect(
					spin_box, SIGNAL(valueChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, spin_box);

				//! Note
				spin_box = new QSpinBox(); spin_box->setProperty("row", row);
				spin_box->setRange(0,127); spin_box->setValue((*it)->t.note);
				connect(
					spin_box, SIGNAL(valueChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, spin_box);

				//! MinNote
				spin_box = new QSpinBox(); spin_box->setProperty("row", row);
				spin_box->setRange(0,127); spin_box->setValue((*it)->t.min_note);
				connect(
					spin_box, SIGNAL(valueChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, spin_box);

				//! MaxNote
				spin_box = new QSpinBox(); spin_box->setProperty("row", row);
				spin_box->setRange(0,127); spin_box->setValue((*it)->t.max_note);
				connect(
					spin_box, SIGNAL(valueChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, spin_box);

				//! MinVelocity
				spin_box = new QSpinBox(); spin_box->setProperty("row", row);
				spin_box->setRange(0,127); spin_box->setValue((*it)->t.min_velocity);
				connect(
					spin_box, SIGNAL(valueChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, spin_box);

				//! MaxVelocity
				spin_box = new QSpinBox(); spin_box->setProperty("row", row);
				spin_box->setRange(0,127); spin_box->setValue((*it)->t.max_velocity);
				connect(
					spin_box, SIGNAL(valueChanged(int)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, spin_box);
				
				//! VelocityFactor
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				double_spin_box->setDecimals(6);
				double_spin_box->setMinimum(-100000.0); double_spin_box->setMaximum(100000.0); double_spin_box->setValue((*it)->t.velocity_factor);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);

				//! AttackG
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				//double_spin_box->setEnabled(false);
				double_spin_box->setDecimals(6);
				double_spin_box->setMinimum(0.0); double_spin_box->setMaximum(100000.0); double_spin_box->setValue((*it)->t.attack_g);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);

				//! DecayG
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				//double_spin_box->setEnabled(false);
				double_spin_box->setDecimals(6);
				double_spin_box->setMinimum(0.0); double_spin_box->setMaximum(100000.0); double_spin_box->setValue((*it)->t.decay_g);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);

				//! SustainG
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				//double_spin_box->setEnabled(false);
				double_spin_box->setDecimals(6);
				double_spin_box->setMinimum(0.0); double_spin_box->setMaximum(100000.0); double_spin_box->setValue((*it)->t.sustain_g);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);

				//! ReleaseG
				double_spin_box = new QDoubleSpinBox(); double_spin_box->setProperty("row", row);
				//double_spin_box->setEnabled(false);
				double_spin_box->setDecimals(6);
				double_spin_box->setMinimum(0.0); double_spin_box->setMaximum(100000.0); double_spin_box->setValue((*it)->t.release_g);
				connect(
					double_spin_box, SIGNAL(valueChanged(double)), this,
					SLOT(generator_cell_widget_changed())
				);
				generator_table->setCellWidget(row, col++, double_spin_box);


				++row;
			}
#endif
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
					disposable_generator_ptr p = disposable_generator::create(
						generator(
							(*it).Name(),
							disposable_sample::create(sample((*it).Sample(), jack_get_sample_rate(engine_.jack_client))),
							(*it).SampleStart(),
							(*it).SampleEnd(),
							(*it).Looping(),
							(*it).Gain(),
							(*it).Channel(),
							(*it).Note(),
							(*it).MinNote(),
							(*it).MaxNote(),
							(*it).MinVelocity(),
							(*it).MaxVelocity(),
							(*it).VelocityFactor(),
							(*it).AttackGain(),
							(*it).DecayGain(),
							(*it).SustainGain(),
							(*it).ReleaseGain()
						));
					l->t.push_back(p);
					log_text_edit->append(QString("Loaded sample: %1").arg((*it).Sample().c_str()));
				}
				write_blocking_command(assign(engine_.gens, l));

				disposable_gvoice_vector_ptr voices(disposable_gvoice_vector::create(std::vector<gvoice>(jass_.Polyphony())));
				write_blocking_command(assign(engine_.voices, voices));

				setup_file_name = file_name;
				deferred_gui_commands.write(boost::bind(&main_window::update_generator_table, this));
				//! Then write them in one go, replacing the whole gens collection
			} catch(...) {
				log_text_edit->append(("something went wrong loading file: " + file_name + ". Try fixing your filesystem mounts, etc, then try reloading the setup").c_str());
			}
		}
	
		void check_acknowledgements() {
			while(engine_.acknowledgements.can_read()) { 
				engine_.acknowledgements.read(); 
				--outstanding_acks; 
			}

			assert(outstanding_acks >= 0);

			if (outstanding_acks == 0) {
				while(deferred_gui_commands.can_read()) deferred_gui_commands.read()();
				setEnabled(true);
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
				std::cout << "current row: " << generator_table->currentRow() << std::endl;
				disposable_generator_list_ptr l = disposable_generator_list::create(engine_.gens->t);
				generator_list::iterator it = l->t.begin();
				std::advance(it, generator_table->currentRow());
				l->t.erase(it);
				write_blocking_command(assign(engine_.gens, l));
				deferred_gui_commands.write(boost::bind(&main_window::update_generator_table, this));
			}
		}

		void generator_cell_widget_changed(void) {
#if 0
			int row = sender()->property("row").toInt();

			generator_list::iterator i = engine_.gens->t.begin();
			std::advance(i, row);

			int col = 2;
			write_command(assign((*i)->t.sample_start, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.sample_end, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.looping, (((QCheckBox*)generator_table->cellWidget(row, col++))->isChecked())));
			write_command(assign((*i)->t.gain, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));

			write_command(assign((*i)->t.channel, (((QSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.note, (((QSpinBox*)generator_table->cellWidget(row, col++))->value())));

			write_command(assign((*i)->t.min_note, (((QSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.max_note, (((QSpinBox*)generator_table->cellWidget(row, col++))->value())));

			write_command(assign((*i)->t.min_velocity, (((QSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.max_velocity, (((QSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.velocity_factor, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));

			write_command(assign((*i)->t.attack_g, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.decay_g, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.sustain_g, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));
			write_command(assign((*i)->t.release_g, (((QDoubleSpinBox*)generator_table->cellWidget(row, col++))->value())));
#endif
		}

		void generator_item_changed(QTableWidgetItem *i) {
#if 0
			int row = i->row();
			generator_list::iterator it = engine_.gens->t.begin();
			std::advance(it, row);
			std::cout << "current row " << row << std::endl;

			write_command(assign((*it)->t.name,std::string(generator_table->item(row,0)->text().toLatin1())));
#endif
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
			log_text_edit->append("In the file browser: Shift-DoubleClick to audit a sample. DoubleClick to load it. Note that you can also select multiple files and then press the Open button to load several samples at once.");
			log_text_edit->append("Use the Parameter menu to quickly-mass assign parameters to selected ranges of generators.");
			log_text_edit->append("Set Continous notes: Set the note of a generator. Select a range of generators by clicking on their names or dragging across their names. Then use the function from the menu. Continous notes will be assigned to all generators, starting with the note of the first in the selection. Also their min and max notes will be set to that same note.");
			log_text_edit->append("-------------------");
		}

	public:
		main_window(engine &e) :
			outstanding_acks(0),
			engine_(e),
			deferred_gui_commands(1024)
		{
			setWindowTitle("jass - jack simple sampler");

			QMenuBar *menu_bar = new QMenuBar();				

				QMenu *file_menu = new QMenu("&File");
				menu_bar->addMenu(file_menu);
					connect(file_menu->addAction("&Open..."), SIGNAL(triggered(bool)), this, SLOT(load_setup()));
					file_menu->addSeparator();
					connect(file_menu->addAction("&Save"), SIGNAL(triggered(bool)), this, SLOT(save_setup()));
					connect(file_menu->addAction("Save &As..."), SIGNAL(triggered(bool)), this, SLOT(save_setup_as()));
					file_menu->addSeparator();
					connect(file_menu->addAction("&Quit"), SIGNAL(triggered(bool)), this, SLOT(close()));

				QMenu *generator_menu = new QMenu("&Generator");
				menu_bar->addMenu(generator_menu);
					generator_menu->addAction("&Duplicate");
					generator_menu->addSeparator();
					connect(generator_menu->addAction("&Remove"), SIGNAL(triggered(bool)), this, SLOT(remove_generator()));;

				QMenu *parameter_menu = new QMenu("&Parameter");
				menu_bar->addMenu(parameter_menu);
					//parameter_menu->addAction("Set &Channel");
					//parameter_menu->addAction("Set &Note");
					//parameter_menu->addAction("Set &Min. Note");
					//parameter_menu->addAction("Set &Max. Note");
					//parameter_menu->addSeparator();
					connect(parameter_menu->addAction("Set continous Notes"), SIGNAL(triggered(bool)), this, SLOT(set_continous_notes()));
					
				QMenu *help_menu = new QMenu("&Help");
				menu_bar->addMenu(help_menu);
					connect(help_menu->addAction("&Help in Log"), SIGNAL(triggered(bool)), this, SLOT(show_help_text()));
					connect(help_menu->addAction("&About"), SIGNAL(triggered(bool)), this, SLOT(show_about_text()));
	
			setMenuBar(menu_bar);

			generator_table = new QTableWidget();
			generator_table->setColumnCount(1);
			QStringList headers;
			headers 
				<< "Generator";
	
			generator_table->setHorizontalHeaderLabels(headers);

#if 0
			//! If you change the headers, make sure you adapt also the functions update_generator_table, generator_item_changed and generator_cell_widget_changed to reflect the new indexes
			generator_table->setColumnCount(17);
			QStringList headers;
			headers 
				<< "Name"
				<< "Sample" 
				<< "Start"
				<< "End"
				<< "Looping"
				<< "Gain" 
				<< "Ch." 
				<< "Note."
				<< "Min. Note" 
				<< "Max. Note" 
				<< "Min. Vel." 
				<< "Max. Vel." 
				<< "Vel. Factor"
				<< "Attack(G)"
				<< "Decay(G)"
				<< "Sustain(G)"
				<< "Release(G)";
	

			generator_table->setHorizontalHeaderLabels(headers);
			connect(generator_table, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(generator_item_changed(QTableWidgetItem*)));
#endif
			setCentralWidget(generator_table);

			file_dialog_dock_widget = new QDockWidget();
			file_dialog_dock_widget->setObjectName("FileDialogDockWidget");
			file_dialog = new QFileDialog(this, Qt::SubWindow);
			file_dialog->setOption(QFileDialog::DontUseNativeDialog);
			file_dialog->setFileMode(QFileDialog::ExistingFiles);
			
			connect(file_dialog, SIGNAL(finished(int)), file_dialog, SLOT(open()));
			connect(file_dialog, SIGNAL(finished(int)), this, SLOT(load_sample_file()));
			connect(file_dialog, SIGNAL(finished(int)), this, SLOT(audit_sample_file()));

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
		}
};

#endif
