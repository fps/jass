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

void test_stuff() {
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

	//boost::dynamic_pointer_cast<disposable<foo> >(ptr)->t.print_x();

	#if 0 	
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

	std::cout << "replacing disposable_generator_vector" << std::endl;
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
	std::cout << p->t.low_velocity << std::endl;

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

	//using namespace boost::phoenix;
	//using namespace boost::phoenix::arg_names;

	#endif

#if 0
	std::cout << "madness" << std::endl;
	disposable_generator_ptr e = gens->t[0];
	// std::cout << "x " << p->t.low_velocity << std::endl;
	// p->t.low_velocity = 234;

	disposable_generator_ptr p = disposable_generator::create(generator());
	p->t.low_velocity  = 322;

	rb.write(assign(gens->t[0], p));
	rb.read()();

	std::cout << "x " << gens->t[0]->t.low_velocity << std::endl;
#endif


	std::cout << "done" << std::endl;
}