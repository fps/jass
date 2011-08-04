#ifndef DISPOSABLE_HH
#define DISPOSABLE_HH

#include "disposable_base.h"
#include "heap.h"

#include <iostream>

template <class T>
struct disposable : public disposable_base {
	const T t;

	static boost::shared_ptr<disposable<T> > create(const T& t = T()) {
		return boost::shared_ptr<disposable<T> >(heap::get()->add(boost::shared_ptr<disposable<T> >(new disposable<T>(t))));
	}

	private:
		disposable(const T &t = T()) : t(t) { 
			std::cout << "disposable" << std::endl;
		}
};


#endif
