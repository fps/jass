#ifndef Q_FUNCTOR_HH
#define Q_FUNCTOR_HH

#include <QObject>
#include <QTimer>

#include <boost/function.hpp>

class qfunctor : public QObject {
	Q_OBJECT

	boost::function<void(void)> f;

	public:
		qfunctor(boost::function<void(void)> t) 
		: 
			f(t) 
		{ 

		}

	public slots:
		void exec() {
			f();
		}
};


#endif
