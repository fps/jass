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

//! a fixed maximum number of generators..
//std::vector<disposable_generator_ptr> generators(128);
disposable_generator_ptr generators[128];

typedef std::vector<
	disposable_generator_ptr
> generator_vector;

typedef
disposable<
	generator_vector
> disposable_generator_vector;

typedef
boost::shared_ptr<
	disposable_generator_vector
> disposable_generator_vector_ptr;

//! disposable vector holding generators
disposable_generator_vector_ptr gens = disposable_generator_vector::create(generator_vector(128));

typedef 
ringbuffer<
		boost::function<void(void)> 
> command_ringbuffer;

//! The ringbuffer for the commands that have to be passed to the process callback
command_ringbuffer rb(1);

bool quit = false;

void signal_handler(int sig) {
	std::cout << "got signal to quit" << std::endl;
	quit = true;
}

int main(int argc, char **argv) {
	//! Make sure the heap instance is created
	heap::get();

	//! Set up signal handler so we can cleanup nicely
	signal(2, signal_handler);

	#if 0
		test_stuff();
	#endif
	
	for (int i = 0; i < 5; ++i) {
		std::cout << "+++++++++++++++++++++++++" << std::endl;
		generator g;

		g.low_velocity = 111;

		rb.write(
			boost::bind(
				&disposable_generator_ptr::operator=<disposable_generator>, 
				&generators[0], 
				disposable_generator::create(g)
			)
		);

		std::cout << "read <-" << std::endl;
		rb.read()();
		std::cout << "read ->" << std::endl;
	}

	std::cout << "velocity_low " << generators[0]->t.low_velocity << std::endl;

	rb.write(
		boost::bind(
			&disposable_generator_vector_ptr::operator=<disposable_generator_vector>,
			&gens, 
			disposable_generator_vector::create(generator_vector(128))
		)
	);
	std::cout << "read <-" << std::endl;
	rb.read()();
	std::cout << "read ->" << std::endl;

	disposable_generator_ptr p = disposable_generator::create(generator());

	rb.write(
		boost::bind(
			&disposable_generator_ptr::operator=<disposable_generator>, 
			gens->t[0], 
			p
		)
	);
	std::cout << "read <-" << std::endl;
	rb.read()();
	std::cout << "read ->" << std::endl;



	while(!quit) {
		heap::get()->cleanup();
		sleep(1);
	}

	std::cout << "exiting" << std::endl;

	delete heap::get();

	return 0;
}