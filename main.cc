#include <jack/jack.h>
#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mem_fn.hpp>

#include <functional>


#include "signal.h"

#include "sample.h"
#include "ringbuffer.h"
#include "disposable.h"
#include "generator.h"

void test_stuff();

//! a fixed maximum number of generators..
//std::vector<disposable_generator_ptr> generators(128);
disposable_generator_ptr generators[128];


//! The ringbuffer for the commands that have to be passed to the process callback
typedef 
ringbuffer<
		boost::function<void(void)> 
> command_ringbuffer;
command_ringbuffer rb(100);

bool quit = false;

void signal_handler(int sig) {
	std::cout << "got signal to quit" << std::endl;
	quit = true;
}

template <class T>
struct foo {
	T t;
};

int main(int argc, char **argv) {
	//! Make sure the heap instance is created
	heap *h = heap::get();

	//! Set up signal handler so we can cleanup nicely
	signal(2, signal_handler);

	#if 0
		test_stuff();
	#endif
	
	{
	generator g;
	g.low_velocity = 111;

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	g.low_velocity = 110;

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	g.low_velocity = 109;

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	g.low_velocity = 108;

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable<generator> >, 
			&generators[0], 
			disposable<generator>::create(g)
		)
	);

	rb.read()();

	}
	std::cout << "velocity_low " << generators[0]->t.low_velocity << std::endl;

	while(!quit) {
		std::cout << "clean" << std::endl;
		heap::get()->cleanup();
		sleep(1);
	}

	std::cout << "exiting" << std::endl;
	delete heap::get();
	return 0;
}