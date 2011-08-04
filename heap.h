#ifndef HEAP_HH
#define HEAP_HH

#include <list>

#include "disposable_base.h"

struct heap {
	std::list<disposable_base_ptr> disposables;

	static heap* instance;

	static heap* get() {
		if (instance) return instance;
		return (instance = new heap());
	}

	void cleanup() {

	}

	protected:
		heap() { }
};


#endif
