#ifndef HEAP_HH
#define HEAP_HH

#include <list>
#include <iostream>

#include "disposable_base.h"

struct heap {
	std::list<disposable_base_ptr> disposables;

	static heap* instance;

	static heap* get() {
		if (instance) return instance;
		return (instance = new heap());
	}

	disposable_base_ptr add(disposable_base_ptr d) {
		disposables.push_back(d);
		return d;
	}

	void cleanup() {
		for (std::list<disposable_base_ptr>::iterator it = disposables.begin(); it != disposables.end(); ++it) {
			if (it->unique()) {
				std::cout << "erase" << std::endl;
				it = disposables.erase(it);
			}
		}	
	}

	~heap() { instance = 0; }

	protected:
		heap() { }
};


#endif
