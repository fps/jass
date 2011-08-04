#ifndef DISPOSABLE_HH
#define DISPOSABLE_HH

#include "disposable_base.h"
#include "heap.h"

#include <iostream>

template <class T>
struct disposable : public disposable_base {
	T t;

	static disposable_base_ptr create(const T& t = T()) {
		return disposable_base_ptr(heap::get()->add(disposable_base_ptr(new disposable(t))));
	}

	private:
		disposable(const T &t = T()) : t(t) { 
			std::cout << "disposable" << std::endl;
		}
};


#endif
