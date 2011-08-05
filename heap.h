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
		//std::cout << "heap::add()" << std::endl;
		disposables.push_back(d);
		std::cout << "add, use count " << d.use_count() << " heap size: " << disposables.size() << std::endl;
		return d;
	}

	void cleanup() {
		std::cout << "============================" << std::endl;
		for (std::list<boost::shared_ptr<disposable_base> >::iterator it = disposables.begin(); it != disposables.end();) {
			std::cout << "cleanup use count " << it->use_count() << std::endl;
			if (it->unique()) {
				//std::cout << "erase" << std::endl;
				it = disposables.erase(it);
			} else {
				++it;
			}
		}	
		std::cout << "****************************" << std::endl;
	}

	~heap() { instance = 0; }

	protected:
		heap() { }
};


#endif
