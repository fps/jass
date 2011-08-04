#ifndef HEAP_HH
#define HEAP_HH

#include <list>
#include <iostream>

#include "disposable_base.h"

struct heap {
	std::list<boost::shared_ptr<disposable_base> > disposables;

	static heap* instance;

	static heap* get() {
		if (instance) return instance;
		return (instance = new heap());
	}

	template <class T>
	T add(T d) {
		disposables.push_back(d);
		return d;
	}

	void cleanup() {
		for (std::list<boost::shared_ptr<disposable_base> >::iterator it = disposables.begin(); it != disposables.end(); ++it) {
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
