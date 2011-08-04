#ifndef TIMED_FUNCTOR_HH
#define TIMED_FUNCTOR_HH

#include <QObject>
#include <boost/function.hpp>

class timed_functor : public QObject {
	Q_OBJECT

	boost::function<void(void)> f;

	public:
		timed_functor(boost::function<void(void)> t) : f(t) { }

	public slots:
		void exec() {
			f();
		}
};

#endif
