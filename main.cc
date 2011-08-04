#include <jack/jack.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "sample.h"
#include "ringbuffer.h"
#include "disposable.h"

struct foo  {
	int x;
	foo(int x = -10) : x(x) {
		std::cout << "foo() " << x << std::endl;
	}

	foo(const foo&f) {
		std::cout << "copy: "  << f.x << std::endl;
		x = f.x;
	}

	~foo() {
		std::cout << "~foo: " << x << std::endl;
	}

	foo& operator=(const foo &f) {
		x = f.x;
		std::cout << "=" << std::endl;
		return *this;
	}

	void print_x() {
		std::cout << "x: " << x << std::endl;
	}
};

//typedef disposable<foo> disposable_foo;
//typedef boost::shared_ptr<disposable<foo> > foo_ptr;

int main(int argc, char **argv) {

	{	
		ringbuffer<boost::shared_ptr<foo> > rb(16);

		for (int i = 0; i < 122; ++i) {
			std::cout << "can write: " << rb.can_write() << std::endl;
			if (rb.can_write())
				rb.write(boost::shared_ptr<foo>(new foo(i)));

			std::cout << "can read: " << rb.can_read() << std::endl;
			if (rb.can_read()) {
				boost::shared_ptr<foo> f = rb.read();
				std::cout << f->x << std::endl;
			}
		}
	}

	ringbuffer<disposable_base_ptr> rb2(10);
	{
		std::cout << "." << std::endl;
		disposable_base_ptr ptr(disposable<foo>::create(-99));
		std::cout << ".." << std::endl;
		std::cout << "..." << std::endl;
		disposable_base_ptr ptr2 = ptr;
		//heap::get()->add(ptr);

		std::cout << "write to buffer" << std::endl;

		rb2.write(ptr);
		rb2.write(ptr2);
	}

	//rb2.write(ptr);

	std::cout << "cleanup" << std::endl;

	heap::get()->cleanup();
	delete heap::get();

	//boost::dynamic_pointer_cast<disposable<foo> >(ptr)->t.print_x();

	std::cout << "done" << std::endl;
}