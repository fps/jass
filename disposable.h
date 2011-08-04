#ifndef DISPOSABLE_HH
#define DISPOSABLE_HH

#include "disposable_base.h"
#include "heap.h"

template <class T>
struct disposable : public disposable_base {
	T t;

	disposable(const T &t = T()) : t(t) { 
		heap::get()->disposables.push_back(disposable_base_ptr(this));
	}
};


#endif
