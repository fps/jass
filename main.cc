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

typedef std::vector<disposable_generator_ptr> generator_vector;
typedef disposable<generator_vector> disposable_generator_vector;
typedef boost::shared_ptr<disposable_generator_vector> disposable_generator_vector_ptr;

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

template<class U, class T>
struct assign_fun {
	U &u;
	T &t;

	assign_fun(U &u, T& t) : u(u), t(t) { }

	void operator()() { u = t; }
};

template<class U, class T>
assign_fun<U,T> 
assign(U &u,  T& t) {
	return assign_fun<U, T>(u, t);
}

int main(int argc, char **argv) {
	//! Make sure the heap instance is created
	heap::get();

	//! Set up signal handler so we can cleanup nicely
	signal(2, signal_handler);

	#if 0
		test_stuff();
	#endif

	std::cout << "madness" << std::endl;
	disposable_generator_ptr e = gens->t[0];
	// std::cout << "x " << p->t.low_velocity << std::endl;
	// p->t.low_velocity = 234;

	disposable_generator_ptr p = disposable_generator::create();
	p->t.low_velocity  = 322;
	rb.write(assign(gens->t[0], p));
	rb.read()();
	std::cout << "x " << gens->t[0]->t.low_velocity << std::endl;

	//rb.write(assign(gens->t[0], p));
	//boost::bind(assign(gens->t[0]))
	//int x,y;
	//boost::bind((ref(x).operator=(ref(y))));
	//boost::bind((boost::phoenix::arg_names::_1 = boost::phoenix::arg_names::_2), x, y);
	//boost::bind((boost::phoenix::arg_names::_1 = boost::phoenix::arg_names::_2), gens->t[0], p);

	//rb.write(boost::bind((arg1 = arg2), (gens->t[0],p));

	while(!quit) {
		heap::get()->cleanup();
		sleep(1);
	}

	std::cout << "exiting" << std::endl;

	delete heap::get();

	return 0;
}