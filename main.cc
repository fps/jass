#include <jack/jack.h>
#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "signal.h"

#include "sample.h"
#include "ringbuffer.h"
#include "disposable.h"
#include "generator.h"

void test_stuff();

//! a fixed maximum number of generators..
std::vector<disposable_generator_ptr> generators(128);

ringbuffer<boost::shared_ptr<disposable<boost::function< return_type(param_type1,param_type2,param_type3)>> > > rb;


bool quit = false;

void signal_handler(int sig) {
	quit = true;
}

int main(int argc, char **argv) {
	signal(2, signal_handler);
	test_stuff();
	

	while(!quit) {
		std::cout << "clean" << std::endl;
		heap::get()->cleanup();
		sleep(1);
	}

	delete heap::get();
	std::cout << "exiting" << std::endl;
	return 0;
}