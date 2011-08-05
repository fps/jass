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

#include "engine.h"


int main(int argc, char **argv) {
	QApplication q_application(argc, argv);

	//! Make sure the heap instance is created
	heap *h = heap::get();

	{
		engine e;

		main_window w(e);
		w.show();

		//! Make sure the heap cleanup is called regularly
		qfunctor f(boost::bind(&heap::cleanup, heap::get()));
		QTimer timer;
		timer.setInterval(5000);
		timer.connect(&timer, SIGNAL(timeout()), &f, SLOT(exec()));
		timer.start();

		q_application.exec();
	}
	std::cout << "exiting" << std::endl;

	delete heap::get();

	return 0;
}