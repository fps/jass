#include <jack/jack.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "sample.h"
#include "ringbuffer.h"
#include "disposable.h"


//typedef disposable<foo> disposable_foo;
//typedef boost::shared_ptr<disposable<foo> > foo_ptr;

void test_stuff();

int main(int argc, char **argv) {
	test_stuff();
	
	heap::get()->cleanup();
	delete heap::get();

	//boost::dynamic_pointer_cast<disposable<foo> >(ptr)->t.print_x();

	std::cout << "done" << std::endl;
}