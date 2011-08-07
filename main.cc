#include <iostream>
#include <vector>
#include <functional>


#include <jack/jack.h>

#include <QApplication>
#include <QTimer>

#include "signal.h"

#include "sample.h"
#include "ringbuffer.h"
#include "disposable.h"
#include "generator.h"
#include "assign.h"

#include "main_window.h"
#include "qfunctor.h"
#include "timed_functor.h"

#include "engine.h"

//! A global variable to communicate the receiption of SIGUSR1 to the check_signalled function
bool signalled = false;
void signal_handler(int signum) {
	if (signum == SIGUSR1) {
		signalled = true;
	}
}

//! Called periodically in GUI thread to see whether we received SIGUSR1 (ladish)
void check_signalled(main_window &w) {
	if (signalled) {
		signalled = false;
		w.save_setup();
	}
}

int main(int argc, char **argv) {
	QApplication q_application(argc, argv);

	//! Make sure the heap instance is created
	heap *h = heap::get();

	{
		engine e;

		main_window w(e);
		if (argc > 1) w.load_setup(argv[1]);

		//! The session_signal is received possibly in the process thread thus we need to use a QueuedConnection
		QObject::connect(
			&e, SIGNAL(session_signal(jack_session_event_t*)), 
			&w, SLOT(handle_jack_session_event(jack_session_event_t*)), 
			Qt::QueuedConnection
		);

		//! register SIGUSR1 for ladish session support
		signal(SIGUSR1, signal_handler);

		w.show();

		//! Register a timed function to clean the heap
		timed_functor tf1(boost::bind(&heap::cleanup, heap::get()), 1000);

		//! This function checks for acknowledgements of the engine and reenables the GUI
		timed_functor tf2(boost::bind(&main_window::check_acknowledgements, &w), 100);

		//! This one checks for ladish save signals..
		timed_functor tf3(boost::bind(check_signalled, boost::ref(w)), 100);

		q_application.exec();
	}
	std::cout << "exiting" << std::endl;

	delete heap::get();

	return 0;
}