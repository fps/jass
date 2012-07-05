#include "ringbuffer.h"
#include <iostream>

#include <boost/shared_ptr.hpp>

struct foo {
	foo() { std::cout << "foo()" << std::endl; }
	~foo() { std::cout << "~foo()" << std::endl; }
};

typedef boost::shared_ptr<foo> foo_ptr;

int main() {
	foo_ptr fp1(new foo());
	
	ringbuffer<foo_ptr> rb(10);

	rb.write(fp1);

	foo_ptr fp2 = rb.read();
}

