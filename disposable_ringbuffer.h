#include "ringbuffer.h"

#include <vector>

template<class T>
struct disposable_ringbuffer : public ringbuffer<boost::shared_ptr<T> > {
	std::vector<T> refs;

	void dispose() {
		for (unsigned int i = 0; i < refs.size(); ++i) 
			if (refs[i].unique()) refs[i] = boost::shared_ptr<T>();
	}

	disposable_ringbuffer(unsigned int size) : ringbuffer<boost::shared_ptr<T> >(size) {

	} 
};
