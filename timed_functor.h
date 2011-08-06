#ifndef JASS_TIMED_FUNCTOR_HH
#define JASS_TIMED_FUNCTOR_HH

struct timed_functor {
	qfunctor qf;
	QTimer timer;
	timed_functor(boost::function<void(void)> t, unsigned int timeout) 
	: 
		qf(t) 
	{
		timer.setInterval(timeout);
		timer.connect(&timer, SIGNAL(timeout()), &qf, SLOT(exec()));
		timer.start();
	}
};

#endif
